| File                |               MD5                |            Version            |
|:--------------------|:--------------------------------:|:-----------------------------:|
| libFLAC-8.dll       | fd74cb249fd04ed439126c4e5d776539 | Bundled with SDL2_mixer 2.0.4 |
| libmodplug-1.dll    | 375b63e3990baa045ae9316bcf8622a6 | Bundled with SDL2_mixer 2.0.4 |
| libmpg123-0.dll     | 91076302dcebf1af925cb8ff2211d56a | Bundled with SDL2_mixer 2.0.4 |
| libogg-0.dll        | 25229b27a1c0c27a14c1afbc53109902 | Bundled with SDL2_mixer 2.0.4 |
| libpng16-16.dll     | 66c3a05a3b728480734671b83028f5c4 |        MSYS2 1.6.37-6         |
| libsamplerate-0.dll | 760e91fe4d4dde22deacc69e9eef683f |    Official release 0.2.2     |
| libvorbis-0.dll     | 7b12b6881e95378a47f60e30b2aade6d | Bundled with SDL2_mixer 2.0.4 |
| libvorbisfile-3.dll | ee01de3f148b0912b88d79d6b34ae3d1 | Bundled with SDL2_mixer 2.0.4 |
| SDL2.dll            | 8cefc3cbc917011cefacc145945f0bbe |            2.0.20             |
| SDL2_mixer.dll      | a355c384e67aaaa11ecf4bf87b2c7bd1 |  Built from git <sup>1</sup>  |
| SDL2_net.dll        | 4a73bf460ffba869fd68f4eca9e0b138 |             2.0.1             |
| zlib1.dll           | 75365924730b0b2c1a6ee9028ef07685 |        MSYS2 1.2.11-9         |

1. We require SDL_mixer binary of at least
   [1c00927](https://github.com/libsdl-org/SDL_mixer/commit/1c0092787398097360f7da745c7644fd32697f3b) revision.
   Built revision [6845d9f](https://github.com/libsdl-org/SDL_mixer/commit/6845d9f3cb3f35542f1ec7e74dba6b30bf968959)
   in [MSYS2](https://www.msys2.org/) environment. Configured with next commands:
   ``` bash
   ./autogen.sh --host=x86_64-w64-mingw64
   
   ./configure \
   --enable-music-wave \
   --enable-music-mod \
   --enable-music-mod-modplug \
   --enable-music-mod-modplug-shared \
   --enable-music-midi \
   --enable-music-midi-timidity \
   --enable-music-midi-native \
   --enable-music-ogg \
   --enable-music-ogg-shared \
   --enable-music-flac \
   --enable-music-flac-shared \
   --enable-music-mp3 \
   --enable-music-mp3-mpg123 \
   --enable-music-mp3-mpg123-shared \
   --disable-music-midi-fluidsynth \
   --disable-music-opus
   ```
