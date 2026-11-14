# Проверка Godot в GitHub Codespaces

Открой терминал Codespaces в корне репозитория и выполни:

```bash
cd SupermanSkyward
bash tools/install_godot.sh
bash tools/validate_godot.sh
```

Установка использует официальный Linux x86_64 релиз Godot 4.3 в `~/.local/bin/godot`, не требует sudo и не меняет существующий мод GTA V.

**Важно:** Headless проверяет импорт и ошибки, но не визуальную картинку. Для Android проверяйте графику в Godot на планшете. Установка/проверка не запускалась автоматически в Codespaces: эти команды необходимо выполнить в его терминале. Если репозиторий обновлялся извне, сначала выполните `git pull origin main`.
