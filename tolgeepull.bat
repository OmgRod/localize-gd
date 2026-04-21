@echo off
for /f "tokens=1,2 delims== eol=#" %%A in (.env) do (
    if /i "%%A"=="TOLGEE_KEY_GD" set TOLGEE_KEY_GD=%%B
    @REM if /i "%%A"=="TOLGEE_KEY_GEODE" set TOLGEE_KEY_GEODE=%%B
    if /i "%%A"=="TOLGEE_KEY_MODS" set TOLGEE_KEY_MODS=%%B
)

tolgee pull --api-key=%TOLGEE_KEY_GD% --project-id=30619 --path ./translations/gd

@REM tolgee pull --api-key=%TOLGEE_KEY_GEODE% --project-id=30620 --path ./translations/geode

tolgee pull --api-key=%TOLGEE_KEY_MODS% --project-id=30621 --path ./translations/mods