const commands = {
    electronX64: 'npm run prebuild:master -- --strip --target 20.0.0 --target 21.0.0 --target 22.0.0 --target 23.0.0 --target 24.0.0 --target 25.0.0 --target 26.0.0 --target 27.0.0 --target 28.0.0 --target 29.0.0 --target 30.0.0 --target 31.0.0 --target 32.0.0 --target 33.0.0 --target 34.0.0 --runtime electron  --arch x64',
    electronIa32: 'npm run prebuild:master -- --strip --target 20.0.0 --target 21.0.0 --target 22.0.0 --target 23.0.0 --target 24.0.0 --target 25.0.0 --target 26.0.0 --target 27.0.0 --target 28.0.0 --target 29.0.0 --target 30.0.0 --target 31.0.0 --target 32.0.0 --target 33.0.0 --target 34.0.0 --runtime electron  --arch ia32',
    nodeIa32: 'npm run prebuild:master -- --strip --target 18.20.6 --target 19.0.0 --target 20.0.0 --target 21.0.0 --target 22.0.0 --arch ia32',
    nodeX64: 'npm run prebuild:master -- --strip --target 18.20.6 --target 19.0.0 --target 20.0.0 --target 21.0.0 --target 22.0.0 --arch x64',
};

module.exports = { commands };