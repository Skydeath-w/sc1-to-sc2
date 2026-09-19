// sc1tosc2 - direct SC1 -> SC2 converter (no Adobe Animate, no .fla).
// Uses the same SupercellFlash library that sc2fla and the SupercellSWF-Animate
// plugin use: load() reads SC1 (+ its *_tex.sc), save_sc2() writes SC2.
//
// Usage:
//   sc1tosc2 <file.sc | folder> [-o <out_dir>] [--ktx]
//
// After saving, every result is loaded back and compared with the source
// (shapes / movieclips / textfields / textures / exports) and reported PASS/FAIL.

#include <flash/objects/SupercellSWF.h>
#include <flash/objects/SWFTexture.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace sc::flash;

namespace {

struct Options {
    fs::path input;
    fs::path outDir = "output";
    bool ktx = false;
};

std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char) std::tolower(c); });
    return s;
}

// *_tex.sc & co are companion texture files, they are pulled in by the main file.
bool isCompanion(const fs::path& p) {
    const std::string stem = lower(p.stem().string());
    static const char* suffixes[] = {"_tex", "_lowres", "_tex_lowres", "_dl", "_highres"};
    for (const char* s : suffixes) {
        const std::string suf = s;
        if (stem.size() >= suf.size() && stem.compare(stem.size() - suf.size(), suf.size(), suf) == 0) return true;
    }
    return false;
}

// SC2 keeps textures inside the file: drop every "external texture" flag.
// Written with requires-expressions so a renamed member in a newer library
// revision does not break the build.
template <typename Swf>
void makeSingleFile(Swf& swf) {
    if constexpr (requires { swf.use_external_texture; }) swf.use_external_texture = false;
    if constexpr (requires { swf.use_external_textures; }) swf.use_external_textures = false;
    if constexpr (requires { swf.compress_external_textures; }) swf.compress_external_textures = false;
    if constexpr (requires { swf.use_low_resolution; }) swf.use_low_resolution = false;
    if constexpr (requires { swf.use_multi_resolution; }) swf.use_multi_resolution = false;
    if constexpr (requires { swf.use_texture_streaming; }) swf.use_texture_streaming = false;
}

// Same call the SupercellSWF-Animate plugin makes on every draw command before save_sc2().
template <typename Swf>
void sortVerticesForSc2(Swf& swf) {
    for (auto& shape : swf.shapes) {
        for (auto& command : shape.commands) {
            if constexpr (requires { command.sort_advanced_vertices(true); }) {
                command.sort_advanced_vertices(true);
            }
        }
    }
}

template <typename Swf>
void useKtxTextures(Swf& swf) {
    for (auto& texture : swf.textures) {
        if constexpr (requires { texture.encoding(SWFTexture::TextureEncoding::KhronosTexture); }) {
            texture.encoding(SWFTexture::TextureEncoding::KhronosTexture);
        }
    }
}

struct Counts {
    size_t shapes, movieclips, textfields, textures, exports;
    bool operator==(const Counts& o) const {
        return shapes == o.shapes && movieclips == o.movieclips && textfields == o.textfields &&
               textures == o.textures && exports == o.exports;
    }
};

template <typename Swf>
Counts countsOf(const Swf& swf) {
    return Counts{(size_t) swf.shapes.size(), (size_t) swf.movieclips.size(), (size_t) swf.textfields.size(),
                  (size_t) swf.textures.size(), (size_t) swf.exports.size()};
}

void print(const char* label, const Counts& c) {
    std::cout << "    " << label << ": shapes=" << c.shapes << " movieclips=" << c.movieclips
              << " textfields=" << c.textfields << " textures=" << c.textures << " exports=" << c.exports << "\n";
}

bool convertOne(const fs::path& in, const fs::path& out, const Options& opt) {
    std::cout << "[..] " << in.string() << "\n";

    Counts before{};
    try {
        SupercellSWF swf;
        swf.load(in);
        before = countsOf(swf);
        print("source", before);

        makeSingleFile(swf);
        sortVerticesForSc2(swf);
        if (opt.ktx) useKtxTextures(swf);

        fs::create_directories(out.parent_path());
        swf.save_sc2(out);
    } catch (const std::exception& e) {
        std::cerr << "[FAIL] " << in.string() << ": " << e.what() << "\n";
        return false;
    }

    // Verification: read the freshly written SC2 back with the same library.
    try {
        SupercellSWF check;
        check.load(out);
        const Counts after = countsOf(check);
        print("result", after);
        if (!(before == after)) {
            std::cerr << "[FAIL] object counts differ after conversion: " << out.string() << "\n";
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "[FAIL] written file can not be read back: " << out.string() << ": " << e.what() << "\n";
        return false;
    }

    std::error_code ec;
    const auto size = fs::file_size(out, ec);
    std::cout << "[PASS] " << out.string() << " (" << (ec ? 0 : (size + 1023) / 1024) << " KB)\n";
    return true;
}

void usage() {
    std::cout << "sc1tosc2 <file.sc | folder> [-o out_dir] [--ktx]\n"
                 "  -o <dir>  output folder (default: output)\n"
                 "  --ktx     re-encode textures as KTX (default: keep raw pixel data)\n";
}

}  // namespace

int main(int argc, char** argv) {
    Options opt;
    for (int i = 1; i < argc; i++) {
        const std::string a = argv[i];
        if (a == "-h" || a == "--help") { usage(); return 0; }
        else if (a == "-o" && i + 1 < argc) opt.outDir = argv[++i];
        else if (a == "--ktx") opt.ktx = true;
        else if (opt.input.empty()) opt.input = a;
        else { std::cerr << "Unknown argument: " << a << "\n"; usage(); return 2; }
    }
    if (opt.input.empty()) { usage(); return 2; }
    if (!fs::exists(opt.input)) { std::cerr << "Not found: " << opt.input.string() << "\n"; return 2; }

    std::vector<std::pair<fs::path, fs::path>> jobs;  // input, output
    if (fs::is_directory(opt.input)) {
        for (const auto& entry : fs::recursive_directory_iterator(opt.input)) {
            if (!entry.is_regular_file()) continue;
            if (lower(entry.path().extension().string()) != ".sc") continue;
            if (isCompanion(entry.path())) continue;
            jobs.emplace_back(entry.path(), opt.outDir / fs::relative(entry.path(), opt.input));
        }
    } else {
        jobs.emplace_back(opt.input, opt.outDir / opt.input.filename());
    }

    if (jobs.empty()) { std::cerr << "No .sc files found.\n"; return 2; }

    size_t ok = 0, failed = 0;
    for (const auto& [in, out] : jobs) {
        std::error_code ec;
        if (fs::exists(out) && fs::equivalent(in, out, ec)) {
            std::cerr << "[SKIP] output equals input: " << in.string() << "\n";
            failed++;
            continue;
        }
        (convertOne(in, out, opt) ? ok : failed)++;
    }
    std::cout << "\nDone: " << ok << " converted, " << failed << " failed.\n";
    return failed ? 1 : 0;
}
