# sc1tosc2 - прямая конвертация SC1 -> SC2 (без Adobe Animate)

Использует ту же библиотеку SupercellFlash, что и sc2fla / плагин SupercellSWF-Animate:
`load()` читает SC1 вместе с `_tex.sc`, `save_sc2()` пишет один SC2-файл.
После записи каждый файл читается обратно и сверяется с исходником
(shapes / movieclips / textfields / textures / exports) - в консоли будет `[PASS]` или `[FAIL]`.

## Вариант 1 - без установки чего-либо (GitHub Actions)
1. Создай на GitHub пустой репозиторий и загрузи в него **всё содержимое этой папки**
   (включая скрытую папку `.github` и папку `input` с loading.sc + loading_tex.sc).
2. Открой вкладку **Actions** - сборка запустится сама (или **Run workflow**). Занимает ~10-20 минут.
3. Когда пройдёт, внизу страницы запуска в **Artifacts** скачай `sc2-result`:
   - `output/loading.sc` - основной вариант (текстуры как есть);
   - `output_ktx/loading.sc` - запасной вариант (текстуры в KTX), если основной не понравится игре.
   Если веб-интерфейс GitHub не принял скрытую папку `.github`: нажми **Add file -> Create new file**,
   впиши путь `.github/workflows/convert.yml` и вставь туда текст из `convert.yml.copy.txt`.
4. Если сборка красная - открой упавший шаг, скопируй текст ошибки и пришли мне.

## Вариант 2 - на своём Windows-ПК
Нужны Visual Studio 2022 Build Tools (компонент C++), CMake, Ninja и Git.
Открой **x64 Native Tools Command Prompt for VS 2022** в этой папке и запусти `build.bat`.
Результат: `output\loading.sc` (и `output_ktx\loading.sc`).

## Свои файлы
Положи SC1-файлы (основной `.sc` вместе с его `_tex.sc`) в `input\` и запусти заново.
Вручную: `build\sc1tosc2.exe input -o output [--ktx]` (можно указать и один файл, и папку).
