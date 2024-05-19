#include <SDL.h>
#include <SDL_mixer.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

// Hàm để hiển thị thông tin bài hát
void displaySongInfo(const std::string &filename) {
    TagLib::FileRef f(filename.c_str());
    if(!f.isNull() && f.tag()) {
        TagLib::Tag *tag = f.tag();
        std::cout << "Title: " << tag->title() << std::endl;
        std::cout << "Artist: " << tag->artist() << std::endl;
        std::cout << "Album: " << tag->album() << std::endl;
        std::cout << "Year: " << tag->year() << std::endl;
    } else {
        std::cerr << "Failed to load tag info for " << filename << std::endl;
    }
}

// Hàm để tải danh sách bài hát từ một thư mục
std::vector<std::string> loadPlaylist(const std::string &directory) {
    std::vector<std::string> playlist;
    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".mp3") {
            playlist.push_back(entry.path().string());
        }
    }
    return playlist;
}

int main() {
    std::string musicDirectory = "/home/ubuntu/toan/taglib/test/mp3";
    std::vector<std::string> playlist = loadPlaylist(musicDirectory);
    if (playlist.empty()) {
        std::cerr << "No mp3 files found in " << musicDirectory << std::endl;
        return 1;
    }

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Failed to initialize SDL_mixer: " << Mix_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("MP3 Player",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          640, 480,
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return 1;
    }

    int currentTrack = 0;
    bool playing = false;
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_SPACE:
                        if (playing) {
                            Mix_PauseMusic();
                        } else {
                            Mix_ResumeMusic();
                        }
                        playing = !playing;
                        break;
                    case SDLK_RIGHT:
                        Mix_HaltMusic();
                        currentTrack = (currentTrack + 1) % playlist.size();
                        break;
                    case SDLK_LEFT:
                        Mix_HaltMusic();
                        currentTrack = (currentTrack - 1 + playlist.size()) % playlist.size();
                        break;
                }
            }
        }

        if (!Mix_PlayingMusic()) {
            Mix_Music *music = Mix_LoadMUS(playlist[currentTrack].c_str());
            if (music) {
                displaySongInfo(playlist[currentTrack]);
                Mix_PlayMusic(music, 1);
                playing = true;
            } else {
                std::cerr << "Failed to load music: " << Mix_GetError() << std::endl;
                quit = true;
            }
        }

        SDL_RenderClear(renderer);
        // Render code here (e.g., display song info on screen)
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }

    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
