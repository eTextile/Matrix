# eTextile matrix sensor - E256 / OpenFrameworks

### Transforming textiles into an intuitive way to interact with computers

## Requirements

### openframeworks 0.10.1

    https://github.com/openframeworks/openFrameworks/releases/tag/0.10.1

To compile the code, you need to set the following variable in your ~/.bashrc, ~/.zshrc or other equivalent:

    # UPDATE WITH YOUR OWN OPEN FRAMEWORKS PATH:
    export OF_ROOT=/opt/openFrameworks/

    cd ${OF_ROOT}
    git submodule update --init
    git submodule update

The E256-openframeworks program requires the following addons:
 - ofxIO
 - ofxSerial
 - ofxOsc
 - ofxGui

The two 1st addons don't come by default with oFx but you can install them:

    cd ${OF_ROOT}/addons
    git clone -b stable https://github.com/bakercp/ofxIO # stable branche
    git clone https://github.com/bakercp/ofxSerial       # stable branche

## Compiling E256-openframeworks
    make

If you have 4 cores, it's recommended to use them all as it's long:

    make -j4

To accelerate the compilation we use ccache, you can get it with:

    apt-get install ccache  # debian-like OS
    brew install ccache     # OSX

### Running (at least Linux & Mac)
    make RunRelease

## TODO

