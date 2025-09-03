@echo off
REM Script de compilation pour le jeu de Morpion Computer Vision
REM Usage: build.bat [Release|Debug] [chemin_vers_opencv]

setlocal enabledelayedexpansion

echo ========================================
echo    COMPILATION MORPION COMPUTER VISION
echo ========================================

REM Configuration par défaut
set BUILD_TYPE=Release
set OPENCV_PATH=

REM Parser les arguments
if not "%1"=="" set BUILD_TYPE=%1
if not "%2"=="" set OPENCV_PATH=%2

echo Type de build: %BUILD_TYPE%

REM Chercher OpenCV automatiquement si pas spécifié
if "%OPENCV_PATH%"=="" (
    echo Recherche automatique d'OpenCV...
    
    REM Chemins typiques d'installation
    set SEARCH_PATHS=C:\opencv\build C:\Program Files\OpenCV C:\tools\opencv\build
    
    for %%P in (%SEARCH_PATHS%) do (
        if exist "%%P\OpenCVConfig.cmake" (
            set OPENCV_PATH=%%P
            echo OpenCV trouvé dans: %%P
            goto opencv_found
        )
        if exist "%%P\x64\vc*" (
            set OPENCV_PATH=%%P
            echo OpenCV trouvé dans: %%P
            goto opencv_found
        )
    )
    
    echo ERREUR: OpenCV non trouvé automatiquement!
    echo Veuillez spécifier le chemin: build.bat Release C:\chemin\vers\opencv\build
    pause
    exit /b 1
)

:opencv_found
echo Utilisation d'OpenCV: %OPENCV_PATH%

REM Créer le répertoire de build
if not exist build mkdir build
cd build

REM Nettoyer le build précédent si demandé
if "%3"=="clean" (
    echo Nettoyage du build précédent...
    rmdir /s /q * 2>nul
)

REM Configuration CMake
echo.
echo Configuration CMake...
if not "%OPENCV_PATH%"=="" (
    cmake .. -DOpenCV_DIR="%OPENCV_PATH%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
) else (
    cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
)

if %ERRORLEVEL% neq 0 (
    echo ERREUR: Échec de la configuration CMake
    echo.
    echo Solutions possibles:
    echo 1. Vérifiez que CMake est installé et dans le PATH
    echo 2. Vérifiez le chemin d'OpenCV
    echo 3. Installez Visual Studio Build Tools
    pause
    exit /b 1
)

REM Compilation
echo.
echo Compilation...
cmake --build . --config %BUILD_TYPE% --parallel

if %ERRORLEVEL% neq 0 (
    echo ERREUR: Échec de la compilation
    pause
    exit /b 1
)

echo.
echo ========================================
echo    COMPILATION TERMINÉE AVEC SUCCÈS!
echo ========================================

REM Trouver l'exécutable
if exist "%BUILD_TYPE%\MorpionComputerVision.exe" (
    set EXE_PATH=%BUILD_TYPE%\MorpionComputerVision.exe
) else if exist "MorpionComputerVision.exe" (
    set EXE_PATH=MorpionComputerVision.exe
) else (
    echo Exécutable trouvé quelque part dans build/
    set EXE_PATH=
)

if not "%EXE_PATH%"=="" (
    echo Exécutable: %CD%\%EXE_PATH%
    echo.
    
    REM Demander si on veut lancer le jeu
    choice /m "Voulez-vous lancer le jeu maintenant"
    if !ERRORLEVEL! equ 1 (
        echo Lancement du jeu...
        start "" "%EXE_PATH%"
    )
) else (
    echo Recherchez l'exécutable MorpionComputerVision.exe dans le dossier build/
)

echo.
echo Pour relancer: %CD%\%EXE_PATH%
pause