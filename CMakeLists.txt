# CMakeLists.txt pour le jeu de Morpion Computer Vision
cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(MorpionComputerVision VERSION 1.0.0 LANGUAGES CXX)

# Standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuration pour Windows
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    # Désactiver les warnings de sécurité MSVC
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    
    # Configuration pour Release par défaut sur Windows
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# Trouver OpenCV
find_package(OpenCV REQUIRED)

# Si OpenCV n'est pas trouvé automatiquement sur Windows
if(WIN32 AND NOT OpenCV_FOUND)
    # Chemins typiques d'installation d'OpenCV sur Windows
    set(OpenCV_POSSIBLE_PATHS
        "C:/opencv/build"
        "C:/Program Files/OpenCV"
        "C:/tools/opencv/build"
        "$ENV{OPENCV_DIR}"
        "$ENV{OpenCV_DIR}"
    )
    
    foreach(path ${OpenCV_POSSIBLE_PATHS})
        if(EXISTS "${path}")
            set(OpenCV_DIR "${path}")
            find_package(OpenCV REQUIRED PATHS ${path} NO_DEFAULT_PATH)
            break()
        endif()
    endforeach()
endif()

# Vérification qu'OpenCV a été trouvé
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV non trouvé ! Veuillez installer OpenCV ou définir OpenCV_DIR")
endif()

# Afficher les informations OpenCV
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")

# Créer l'exécutable
add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/tictactoe_cv.cpp
    src/grid_detection.cpp
    src/ai.cpp
    src/utils.cpp
)

# Inclure les headers OpenCV
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})

# Lier les bibliothèques OpenCV
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# Configuration spécifique Windows pour éviter la console
if(WIN32)
    # Décommenter la ligne suivante si vous ne voulez pas de console
    # set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    
    # Copier les DLLs OpenCV dans le répertoire de sortie (pour le debug)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Rechercher les DLLs d'OpenCV
        file(GLOB OPENCV_DLLS "${OpenCV_DIR}/x64/vc*/bin/*d.dll")
    else()
        file(GLOB OPENCV_DLLS "${OpenCV_DIR}/x64/vc*/bin/*.dll")
    endif()
    
    # Exclure les DLLs de debug en mode Release et vice versa
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(FILTER OPENCV_DLLS INCLUDE REGEX ".*d\\.dll$")
    else()
        list(FILTER OPENCV_DLLS EXCLUDE REGEX ".*d\\.dll$")
    endif()
    
    # Copier les DLLs
    foreach(dll ${OPENCV_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${dll}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMENT "Copie des DLLs OpenCV"
        )
    endforeach()
endif()

# Afficher des informations utiles
message(STATUS "Configuration terminée!")
message(STATUS "Système: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compilateur: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Type de build: ${CMAKE_BUILD_TYPE}")

# Instructions d'utilisation
if(WIN32)
    message(STATUS "")
    message(STATUS "=== INSTRUCTIONS POUR WINDOWS ===")
    message(STATUS "1. Assurez-vous qu'OpenCV est installé")
    message(STATUS "2. Définissez OpenCV_DIR si nécessaire:")
    message(STATUS "   cmake -DOpenCV_DIR=C:/opencv/build ..")
    message(STATUS "3. Compilez avec:")
    message(STATUS "   cmake --build . --config Release")
    message(STATUS "4. L'exécutable sera dans Release/ ou Debug/")
endif()

# Cible d'installation (optionnelle)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# Empaquetage (optionnel)
set(CPACK_PACKAGE_NAME "MorpionComputerVision")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Jeu de Morpion avec Computer Vision")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)