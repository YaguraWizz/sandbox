cpp-template-engine/
├── CMakeLists.txt              # основной CMake проекта
├── template-engine/            # исходники утилиты
│   ├── main.cpp                # входная точка приложения
│   ├── cli.cpp / cli.hpp       # разбор аргументов и команды
│   ├── generator.cpp / .hpp    # генерация проекта
│   └── CMakeLists.txt          # подмодульный cmake
│
├── template/                   # шаблонные файлы
│   ├── CMakeLists.txt.tpl      # шаблон корневого CMakeLists.txt
│   ├── main.cpp.tpl            # шаблон исходника
│   ├── build.bat.tpl           # шаблон bat-файла
│   └── build.sh.tpl            # шаблон shell-скрипта
│
├── tests/                      # модульные/интеграционные тесты
│   ├── test_cli.cpp
│   └── CMakeLists.txt
│
├── automation/                 # скрипты для разработки
│   ├── setup_env.bat / .sh
│   └── regenerate_project.sh
│
└── README.md








# project.yaml (или template_config.yaml)

project:
  name: MyAwesomeProject
  version: 1.0.0
  description: A short description of the project.
  language: cxx # cxx, c, mixed, python, ...
  authors:
    - John Doe <john.doe@example.com>
    - Jane Smith <jane.smith@example.com>
  license: MIT # Apache-2.0, GPL-3.0, custom, ...

build_system:
  name: cmake
  version: 3.15 # Minimum required CMake version
  # Можно добавить специфические опции CMake, например:
  # options:
  #   - BUILD_SHARED_LIBS=ON
  #   - CMAKE_POSITION_INDEPENDENT_CODE=ON

package_manager:
  name: conan
  version: 2.x # Optional: specify minimum Conan version if needed

conan_profile_defaults: # Эти настройки будут использованы для генерации базового профиля Conan
  settings:
    os: Linux
    arch: x86_64
    compiler: gcc
    compiler.version: "14" # Версии лучше указывать строками
    compiler.cppstd: gnu17
    compiler.libcxx: libstdc++11
    build_type: Release # Можно генерировать профили Debug и Release отдельно, но это хороший дефолт
  # options: # Можно добавить общие опции для всех пакетов, если это нужно
  #   MyPackage: my_option=value
  # env: # Можно добавить переменные окружения для профиля
  #   MY_VAR: "some_value"

dependencies: # Зависимости Conan
  # Формат: "имя/версия#ревизия@пользователь/канал"
  # Или просто "имя/версия" для общедоступных пакетов
  public:
    - openssl/3.3.0
    - zlib/1.3.1
    - fmt/10.2.1
  private: # Если нужны внутренние зависимости, которые не всегда публичны
    - my_internal_lib/1.0@mycompany/stable

# testing: # Возможно, стоит добавить секцию для настроек тестирования
#   framework: googletest # или catch2
#   enabled: true

# docker_support: # Если есть поддержка Docker для dev-среды
#   enabled: true
#   image_name: myproject-dev
#   base_image: ubuntu:22.04


