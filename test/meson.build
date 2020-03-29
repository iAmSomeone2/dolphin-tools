gtest = dependency('GTest')

yaz0test = executable('yaz0test', 'Yaz0/Yaz0Test.cpp',
    dependencies: [gtest],
    link_with: yaz0,
    include_directories: [project_include])

test('Yaz0 Decoder Test', yaz0test)

copyTestResources = run_command('cp','-r', 'resources/', '../build/test/')