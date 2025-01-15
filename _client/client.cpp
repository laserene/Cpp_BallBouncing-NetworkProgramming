#include "cstdio"
#include "cstdlib"
#include "cstring"
#include "unistd.h"
#include "pthread.h"
#include "arpa/inet.h"

#include "stage.h"
#include "common.h"
#include "draw.h"
#include "init.h"
#include "input.h"
#include "client.h"

#include "welcome.h"
#include "auth.h"
#include "menu.h"
#include "leaderboard.h"

App app;
Stage stage;
Screen screen = WELCOME;
// Screen screen = MENU;

char returning[BUFFER_SIZE] = {};
bool disconnected = false;

void handle_server_message(char *buffer, char *returning) {
    if (strncmp(buffer, "AUTH", 4) == 0) {
        sscanf(buffer + 5, "%s", returning);
        if (strcmp(returning, "LOGIN_SUCCESS") == 0) {
            memset(returning, 0, BUFFER_SIZE);
            screen = MENU;
        } else if (strcmp(returning, "REGISTRATION_SUCCESS") == 0) {
            memset(returning, 0, BUFFER_SIZE);
            screen = LOGIN;
        }
    } else if (strncmp(buffer, "LEADERBOARD RETURN", 18) == 0) {
        int mode, score;
        sscanf(buffer + 19, "%s %d %d", buffer, &mode, &score);
    } else if (strncmp(buffer, "UPDATE", 6) == 0) {
        if (player != nullptr) {
            if (strncmp(buffer, "UPDATE", 6) == 0) {
                int u, d, l, r;
                sscanf(buffer, SERVER_UPDATE, &u, &d, &l, &r);
                player->dy += u;
                player->dy += d;
                player->dx += l;
                player->dx += r;

                player->x += player->dx;
                player->y += player->dy;
            } else {
                std::cout << buffer << std::endl;
            }
        }
    }
}

void handle_input(char *buffer) {
    memset(buffer, 0, BUFFER_SIZE);
    SDL_Event event;
    SDL_StartTextInput();
    bool run = true;
    while (run) {
        SDL_PollEvent(&event);
        if (event.type == SDL_TEXTINPUT) {
            std::cout << event.text.text << "\n";
            if (strlen(buffer) + strlen(event.text.text) < BUFFER_SIZE) {
                strcat(buffer, event.text.text);
            }
        } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) run = false;
    }
}

void *receiveData(void *clientSocketPtr) {
    const int clientSocket = *static_cast<int *>(clientSocketPtr);

    // Handling server messages
    while (true) {
        char buffer[1024] = {};
        if (const ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0); bytesReceived <= 0) {
            printf("Disconnected from server.\n");
            close(clientSocket);
            disconnected = true;
            return nullptr;
        }
        handle_server_message(buffer, returning);
    }
}

int main(const int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IPAddress> <PortNumber>\n", argv[0]);
        return -1;
    }

    // Init client socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Socket creation failed");
        return -1;
    }

    // Set up server address
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

    // Connecting
    if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) < 0) {
        perror("Connection to server failed");
        return -1;
    }
    printf("Connected to server.\n");

    // Create a thread for handling server messages
    pthread_t receiverThread;
    pthread_create(&receiverThread, nullptr, receiveData, &clientSocket);
    pthread_detach(receiverThread);

    // Reset game components
    memset(&app, 0, sizeof(App));

    // Init SDL
    initSDL();
    atexit(cleanup);
    initStage();
    initFonts();

    long then = SDL_GetTicks();
    float remainder = 0;

    SDL_Texture *box = loadTexture(BOX_TEXTURE);
    SDL_Texture *half = loadTexture(HALFBOX_TEXTURE);
    SDL_Texture *res = loadTexture(RESPONSE_TEXTURE);

    // SDL_Texture *res = loadTexture(RESPONSE_TEXTURE);

    char account[BUFFER_SIZE] = {};
    char password[BUFFER_SIZE] = {};
    char buffer[BUFFER_SIZE] = {};

    // Game loop
    while (true) {
        prepareScene();
        if (screen == WELCOME) {
            doBackground();
            doStarfield();

            drawBackground();
            drawStarfield();
            blit(half, 660, 200);
            blit(half, 660, 300);
            blit(half, 660, 400);
            drawText(660, 120, 255, 255, 255, WELCOME_TEXT);
            drawText(660, 220, 0, 0, 0, OPTION_LOGIN_TEXT);
            drawText(660, 320, 0, 0, 0, OPTION_SIGNUP_TEXT);
            drawText(660, 420, 0, 0, 0, OPTION_EXIT_TEXT);

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        case SDLK_KP_1:
                            screen = LOGIN;
                            break;
                        case SDLK_KP_2:
                            screen = SIGNUP;
                            break;
                        case SDLK_KP_3:
                        case SDLK_ESCAPE:
                            screen = EXIT;
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        if (screen == LOGIN || screen == SIGNUP) {
            doBackground();
            doStarfield();

            drawBackground();
            drawStarfield();
            blit(box, 680, 200);
            blit(box, 680, 300);
            blit(half, 520, 400);
            blit(half, 800, 400);
            blit(res, 1100, 100);
            if (screen == LOGIN) {
                drawText(680, 120, 255, 255, 255, LOGIN_TEXT);
            } else drawText(680, 120, 255, 255, 255, SIGNUP_TEXT);


            drawText(460, 220, 255, 255, 255, ACCOUNT_TEXT);
            drawText(460, 320, 255, 255, 255, PASSWORD_TEXT);
            drawText(520, 420, 0, 0, 0, ENTER_TEXT);
            drawText(800, 420, 0, 0, 0, RETURN_TEXT);
            drawText(800, 420, 0, 0, 0, RETURN_TEXT);

            drawText(700, 220, 0, 0, 0, account);
            drawText(700, 320, 0, 0, 0, password);
            drawText(1152, 260, 0, 0, 0, returning);

            SDL_Event event;
            bool insertAccount = false;
            bool insertPassword = false;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        // Input account
                        case SDLK_KP_1:
                            insertAccount = true;
                            break;
                        // Input password
                        case SDLK_KP_2:
                            insertPassword = true;
                            break;
                        // Logging in
                        case SDLK_KP_3:
                        case SDLK_RETURN:
                            memset(buffer, 0, BUFFER_SIZE);

                            if (screen == LOGIN) {
                                snprintf(buffer, sizeof(buffer), SEND_LOGIN, account, password);
                            } else snprintf(buffer, sizeof(buffer), SEND_REGISTER, account, password);

                            send(clientSocket, buffer, BUFFER_SIZE, 0);
                            break;
                        case SDLK_KP_4:
                        case SDLK_BACKSPACE:
                        case SDLK_ESCAPE:
                            memset(returning, 0, BUFFER_SIZE);
                            screen = WELCOME;
                            break;
                        default:
                            break;
                    }
                }
            }

            if (insertAccount) {
                handle_input(account);
            } else if (insertPassword) {
                handle_input(password);
            }
        }

        if (screen == MENU) {
            doBackground();
            doStarfield();

            drawBackground();
            drawStarfield();
            blit(box, 600, 100);
            blit(box, 600, 200);
            blit(box, 600, 300);
            blit(box, 600, 400);

            drawText(720, 20, 255, 255, 255, MENU_TEXT);
            drawText(610, 120, 0, 0, 0, OPTION_SINGLE_TEXT);
            drawText(610, 220, 0, 0, 0, OPTION_MULTI_TEXT);
            drawText(610, 320, 0, 0, 0, OPTION_LEADERBOARD_TEXT);
            drawText(610, 420, 0, 0, 0, OPTION_MENU_EXIT_TEXT);

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        // Input account
                        case SDLK_KP_1:
                            // insertAccount = true;
                            memset(buffer, 0,BUFFER_SIZE);
                            screen = PLAY;
                            break;
                        // Input password
                        case SDLK_KP_2:
                            // insertPassword = true;
                            break;
                        case SDLK_KP_3:
                            memset(returning, 0, BUFFER_SIZE);
                            screen = LEADERBOARD;
                            break;
                        case SDLK_KP_4:
                        case SDLK_BACKSPACE:
                        case SDLK_ESCAPE:
                            memset(returning, 0, BUFFER_SIZE);
                            screen = EXIT;
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        if (screen == CHARACTER) {
            doInput();
            app.delegate.logic(clientSocket);
            app.delegate.draw();
        }

        if (screen == BIOME) {
            doInput();
            app.delegate.logic(clientSocket);
            app.delegate.draw();
        }

        if (screen == ROOM) {
            doInput();
            app.delegate.logic(clientSocket);
            app.delegate.draw();
        }

        if (screen == PLAY) {
            doInput();
            app.delegate.logic(clientSocket);
            app.delegate.draw();
        }

        if (screen == LEADERBOARD) {
            doBackground();
            doStarfield();

            drawBackground();
            drawStarfield();
            blit(half, 0, 10);

            drawText(720, 20, 255, 255, 255, LEADERBOARD_TEXT);
            drawText(30, 30, 0, 0, 0, OPTION_RETURN_TEXT);

            drawText(220, 90, 255, 255, 255, HALLOW_TEXT);
            drawText(560, 90, 255, 255, 255, SNOW_TEXT);
            drawText(900, 90, 255, 255, 255, RUSTY_TEXT);
            drawText(1200, 90, 255, 255, 255, SPACE_TEXT);

            // Retrieve leaderboard from server and save to a file.
            // Then read the file and render the data
            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, sizeof(buffer), SEND_LEADERBOARD_GET);
            send(clientSocket, buffer, strlen(buffer), 0);

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        // Input account
                        case SDLK_KP_1:
                        case SDLK_BACKSPACE:
                        case SDLK_ESCAPE:
                            memset(returning, 0, BUFFER_SIZE);
                            screen = MENU;
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        if (screen == EXIT) {
            break;
        }

        presentScene();
        capFrameRate(&then, &remainder);

        if (disconnected) {
            break;
        }
    }

    return 0;
}

static void capFrameRate(long *then, float *remainder) {
    // Set 60 FPS
    long wait = 16 + *remainder;
    *remainder -= static_cast<int>(*remainder);
    const long frameTime = SDL_GetTicks() - *then;

    wait -= frameTime;
    if (wait < 1) {
        wait = 1;
    }

    SDL_Delay(wait);
    *remainder += 0.667;
    *then = SDL_GetTicks();
}
