project('dolphin-tools', ['cpp'],
    version: '0.1',
    default_options: ['warning_level=3', 'cpp_std=c++17'])

project_include = include_directories('include')

boost = dependency('boost', version: '>=1.66.0')

subdir('src/Yaz0')

executable('dolphin-tools', 'src/dolphin-tools.cpp',
    dependencies: [boost],
    link_with: yaz0,
    install: true)

subdir('test')