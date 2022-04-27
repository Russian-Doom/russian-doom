| File                |               MD5                |            Version            |
|:--------------------|:--------------------------------:|:-----------------------------:|
| libFLAC-8.dll       | 41814ba17f65a283d64bacedc8966deb | Bundled with SDL2_mixer 2.0.4 |
| libgcc_s_dw2-1.dll  | e2ac23418781f632311513944edd0a4c |              ???              |
| libmodplug-1.dll    | 1c0ae563c7b3a0404bf69cbf5b960652 | Bundled with SDL2_mixer 2.0.4 |
| libmpg123-0.dll     | a24db14f68919c1acd41cd61d796a27e | Bundled with SDL2_mixer 2.0.4 |
| libogg-0.dll        | 47ecbe810165f34eb2d900326bc1944a | Bundled with SDL2_mixer 2.0.4 |
| libpng16-16.dll     | 41a546b325ddfb0f37b84b550d215aef |        MSYS2 1.6.37-6         |
| libsamplerate-0.dll | 27965bdd3242f1e7f7da667593baeee5 |         MSYS2 0.1.9-1         |
| libvorbis-0.dll     | 638a1c86c660613cfd30bae95d2b2054 | Bundled with SDL2_mixer 2.0.4 |
| libvorbisfile-3.dll | ea20e1550ad37aa978897989bebe3098 | Bundled with SDL2_mixer 2.0.4 |
| SDL2.dll            | a399b08b541a11c56d88f32881231f4f |            2.0.22             |
| SDL2_mixer.dll      | 1d5d1f3d18312a04dd9287e742fc8e50 |  Built from git <sup>1</sup>  |
| SDL2_net.dll        | c1332b71a2f74e35fc6ae892f081272c |             2.0.1             |
| zlib1.dll           | 8b2a6e8419a8a4e7d3fd023d97455fb9 |        MSYS2 1.2.11-9         |

1. We require SDL_mixer binary of at least
   [1c00927](https://github.com/libsdl-org/SDL_mixer/commit/1c0092787398097360f7da745c7644fd32697f3b) revision.
   Built revision [6845d9f](https://github.com/libsdl-org/SDL_mixer/commit/6845d9f3cb3f35542f1ec7e74dba6b30bf968959)
   in [MSYS2](https://www.msys2.org/) environment. Configured with next commands:
   ``` bash
   ./autogen.sh --host=i686-w64-mingw64
   
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
