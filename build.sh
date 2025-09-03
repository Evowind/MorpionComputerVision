#!/bin/bash
set -e

# Création du dossier de build
mkdir -p build
cd build

# Configuration avec CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Compilation
make -j$(nproc)

echo "Compilation terminée!"
echo "Exécutable disponible dans: build/bin/morpion_cv"