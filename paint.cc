#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Global SDL objects
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* canvasTexture = nullptr; 

// Brush settings
int brushSize = 10;
SDL_Color currentColor = {0, 0, 0, 255};
bool rainbowMode = false;

// Tools available for drawing
enum Tool { TOOL_BRUSH, TOOL_ERASER, TOOL_LINE, TOOL_RECTANGLE, TOOL_CIRCLE };
Tool currentTool = TOOL_BRUSH;

// Color palette button structure
struct ColorButton {
    SDL_Rect rect;
    SDL_Color color;
};
std::vector<ColorButton> colorPalette;

// store snapshots of the canvas
std::vector<SDL_Surface*> undoStack;

// Font for status bar text 
TTF_Font* font = nullptr;

// For shape drawing (line, rectangle, circle)
bool isDrawing = false;
bool drawingShape = false;
SDL_Point startPoint = {0, 0};
SDL_Point currentPoint = {0, 0};

// Function prototypes
bool init();
void initCanvas();
bool loadFont();
void initColorPalette();
void renderPalette();
void handleColorSelection(int x, int y);
void saveCanvasState();
void undoLastAction();
SDL_Color hueToRGB(double hue);
void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius);
void drawPreviewShape();
void commitShape();
void renderStatusBar();
void clearCanvas();

int main() {
    if (!init()) return -1;
    if (!loadFont()) return -1;

    initCanvas();
    initColorPalette();

    // Print user instructions to the console
    std::cout << "=== Advanced Paint Instructions ===\n";
    std::cout << "Tools: 1: Brush | 2: Eraser | 3: Line | 4: Rectangle | 5: Circle\n";
    std::cout << "Toggle Rainbow Mode: M\n";
    std::cout << "Adjust Brush Size: +/-\n";
    std::cout << "Save Canvas: S | Load Canvas: O\n";
    std::cout << "Undo: U | Clear Canvas: C\n";
    std::cout << "Exit: ESC\n";
    std::cout << "===================================\n";

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            // Handle quitting events
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_1:
                        currentTool = TOOL_BRUSH;
                        std::cout << "Brush selected\n";
                        break;
                    case SDLK_2:
                        currentTool = TOOL_ERASER;
                        std::cout << "Eraser selected\n";
                        break;
                    case SDLK_3:
                        currentTool = TOOL_LINE;
                        std::cout << "Line selected\n";
                        break;
                    case SDLK_4:
                        currentTool = TOOL_RECTANGLE;
                        std::cout << "Rectangle selected\n";
                        break;
                    case SDLK_5:
                        currentTool = TOOL_CIRCLE;
                        std::cout << "Circle selected\n";
                        break;
                    case SDLK_m:
                        rainbowMode = !rainbowMode;
                        std::cout << "Rainbow mode: " << (rainbowMode ? "ON" : "OFF") << "\n";
                        break;
                    case SDLK_PLUS:
                    case SDLK_KP_PLUS:
                        brushSize += 2;
                        break;
                    case SDLK_MINUS:
                    case SDLK_KP_MINUS:
                        if (brushSize > 2) brushSize -= 2;
                        break;
                    case SDLK_s: { // Save canvas
                        SDL_SetRenderTarget(renderer, canvasTexture);
                        SDL_Surface* saveSurface = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
                        SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, saveSurface->pixels, saveSurface->pitch);
                        SDL_SaveBMP(saveSurface, "canvas.bmp");
                        SDL_FreeSurface(saveSurface);
                        SDL_SetRenderTarget(renderer, NULL);
                        std::cout << "Canvas saved as canvas.bmp\n";
                        break;
                    }
                    case SDLK_o: { // Load canvas
                        SDL_Surface* loadedSurface = SDL_LoadBMP("canvas.bmp");
                        if (!loadedSurface) {
                            std::cerr << "Failed to load canvas.bmp: " << SDL_GetError() << "\n";
                        } else {
                            SDL_Texture* loadedTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
                            SDL_SetRenderTarget(renderer, canvasTexture);
                            SDL_RenderCopy(renderer, loadedTexture, NULL, NULL);
                            SDL_SetRenderTarget(renderer, NULL);
                            SDL_DestroyTexture(loadedTexture);
                            SDL_FreeSurface(loadedSurface);
                            std::cout << "Canvas loaded from canvas.bmp\n";
                        }
                        break;
                    }
                    case SDLK_u:
                        undoLastAction();
                        break;
                    case SDLK_c:
                        clearCanvas();
                        break;
                    default:
                        break;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    int x = e.button.x, y = e.button.y;
                    // Check if a palette color was clicked
                    handleColorSelection(x, y);

                    isDrawing = true;
                    startPoint = {x, y};
                    currentPoint = {x, y};
                    drawingShape = (currentTool == TOOL_LINE ||
                                    currentTool == TOOL_RECTANGLE ||
                                    currentTool == TOOL_CIRCLE);
                    // For immediate drawing tools (brush/eraser), draw one dot on click
                    if (currentTool == TOOL_BRUSH || currentTool == TOOL_ERASER) {
                        SDL_SetRenderTarget(renderer, canvasTexture);
                        if (currentTool == TOOL_BRUSH) {
                            if (rainbowMode) {
                                double hue = SDL_GetTicks() % 360;
                                currentColor = hueToRGB(hue);
                            }
                            SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, 255);
                        } else {  // Eraser draws white
                            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        }
                        SDL_Rect brushRect = { x - brushSize/2, y - brushSize/2, brushSize, brushSize };
                        SDL_RenderFillRect(renderer, &brushRect);
                        SDL_SetRenderTarget(renderer, NULL);
                    }
                }
            }
            else if (e.type == SDL_MOUSEMOTION) {
                if (isDrawing) {
                    currentPoint = { e.motion.x, e.motion.y };
                    // For brush/eraser, draw continuously onto the canvas texture
                    if (currentTool == TOOL_BRUSH || currentTool == TOOL_ERASER) {
                        SDL_SetRenderTarget(renderer, canvasTexture);
                        if (currentTool == TOOL_BRUSH) {
                            if (rainbowMode) {
                                double hue = SDL_GetTicks() % 360;
                                currentColor = hueToRGB(hue);
                            }
                            SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, 255);
                        } else {
                            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        }
                        SDL_Rect brushRect = { e.motion.x - brushSize/2, e.motion.y - brushSize/2, brushSize, brushSize };
                        SDL_RenderFillRect(renderer, &brushRect);
                        SDL_SetRenderTarget(renderer, NULL);
                    }
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    isDrawing = false;
                    if (drawingShape) {
                        // When drawing a shape, commit it to the canvas
                        commitShape();
                        drawingShape = false;
                    }
                    // Save state for undo after finishing a drawing action
                    saveCanvasState();
                }
            }
        } 

  
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, canvasTexture, NULL, NULL);

        if (isDrawing && drawingShape) {
            drawPreviewShape();
        }

        renderPalette();
        renderStatusBar();
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }


    for (auto surface : undoStack) {
        SDL_FreeSurface(surface);
    }
    if (font) TTF_CloseFont(font);
    SDL_DestroyTexture(canvasTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

// Initialize SDL, window, renderer, and TTF
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed! " << SDL_GetError() << "\n";
        return false;
    }
    if (TTF_Init() == -1) {
        std::cerr << "TTF initialization failed! " << TTF_GetError() << "\n";
        return false;
    }
    window = SDL_CreateWindow("Advanced Paint Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed! " << SDL_GetError() << "\n";
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!renderer) {
        std::cerr << "Renderer creation failed! " << SDL_GetError() << "\n";
        return false;
    }
    return true;
}

// Create the canvas texture and fill it with white
void initCanvas() {
    canvasTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                      SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderTarget(renderer, canvasTexture);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, NULL);
}

// Load a TrueType font 
bool loadFont() {
    font = TTF_OpenFont("ARIAL.TTF", 16);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << "\n";
        return false;
    }
    return true;
}

// Initialize a vibrant color palette
void initColorPalette() {
    colorPalette.clear();
    int startX = 10, startY = 10, size = 40, spacing = 10;
    std::vector<SDL_Color> colors = {
        {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255},
        {255, 255, 0, 255}, {255, 0, 255, 255}, {0, 255, 255, 255},
        {128, 0, 128, 255}, {255, 165, 0, 255}, {0, 128, 128, 255},
        {128, 128, 0, 255}
    };
    for (auto col : colors) {
        SDL_Rect rect = { startX, startY, size, size };
        colorPalette.push_back({ rect, col });
        startX += size + spacing;
    }
}

// Draw the color palette buttons on the screen
void renderPalette() {
    for (auto& btn : colorPalette) {
        SDL_SetRenderDrawColor(renderer, btn.color.r, btn.color.g, btn.color.b, 255);
        SDL_RenderFillRect(renderer, &btn.rect);
        // Draw a border around each button
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &btn.rect);
    }
}


void handleColorSelection(int x, int y) {
    SDL_Point point = { x, y };
    for (auto& btn : colorPalette) {
        if (point.x >= btn.rect.x && point.x <= btn.rect.x + btn.rect.w &&
            point.y >= btn.rect.y && point.y <= btn.rect.y + btn.rect.h) {
            currentColor = btn.color;
            rainbowMode = false; // disable rainbow mode when a fixed color is selected
        }
    }
}

// Save the current canvas state to the undo stack
void saveCanvasState() {
    SDL_SetRenderTarget(renderer, canvasTexture);
    SDL_Surface* backup = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, backup->pixels, backup->pitch);
    undoStack.push_back(backup);
    SDL_SetRenderTarget(renderer, NULL);
}

// Undo the last drawing action by restoring the previous canvas state
void undoLastAction() {
    if (!undoStack.empty()) {
        SDL_Surface* backup = undoStack.back();
        undoStack.pop_back();
        SDL_Texture* tempTexture = SDL_CreateTextureFromSurface(renderer, backup);
        SDL_SetRenderTarget(renderer, canvasTexture);
        SDL_RenderCopy(renderer, tempTexture, NULL, NULL);
        SDL_SetRenderTarget(renderer, NULL);
        SDL_DestroyTexture(tempTexture);
        SDL_FreeSurface(backup);
        std::cout << "Undo performed.\n";
    } else {
        std::cout << "Nothing to undo.\n";
    }
}

// Convert a hue (0-360) to an SDL_Color using full saturation and brightness
SDL_Color hueToRGB(double hue) {
    double s = 1.0, v = 1.0;
    double c = v * s;
    double x = c * (1 - fabs(fmod(hue / 60.0, 2) - 1));
    double m = v - c;
    double r, g, b;
    if (hue < 60)       { r = c; g = x; b = 0; }
    else if (hue < 120) { r = x; g = c; b = 0; }
    else if (hue < 180) { r = 0; g = c; b = x; }
    else if (hue < 240) { r = 0; g = x; b = c; }
    else if (hue < 300) { r = x; g = 0; b = c; }
    else                { r = c; g = 0; b = x; }
    SDL_Color color = { static_cast<Uint8>((r + m) * 255),
                        static_cast<Uint8>((g + m) * 255),
                        static_cast<Uint8>((b + m) * 255),
                        255 };
    return color;
}

// Simple circle-drawing function 
void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    int x = radius, y = 0;
    int err = 0;
    while (x >= y) {
        SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
        SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);
        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

// Render a preview of the shape (line, rectangle, circle) while the mouse is dragging
void drawPreviewShape() {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, 128);
    switch (currentTool) {
        case TOOL_LINE:
            SDL_RenderDrawLine(renderer, startPoint.x, startPoint.y, currentPoint.x, currentPoint.y);
            break;
        case TOOL_RECTANGLE: {
            SDL_Rect rect;
            rect.x = std::min(startPoint.x, currentPoint.x);
            rect.y = std::min(startPoint.y, currentPoint.y);
            rect.w = std::abs(startPoint.x - currentPoint.x);
            rect.h = std::abs(startPoint.y - currentPoint.y);
            SDL_RenderDrawRect(renderer, &rect);
            break;
        }
        case TOOL_CIRCLE: {
            int radius = static_cast<int>(std::sqrt((currentPoint.x - startPoint.x) * (currentPoint.x - startPoint.x) +
                                                    (currentPoint.y - startPoint.y) * (currentPoint.y - startPoint.y)));
            drawCircle(renderer, startPoint.x, startPoint.y, radius);
            break;
        }
        default:
            break;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

// Commit the previewed shape to the canvas texture
void commitShape() {
    SDL_SetRenderTarget(renderer, canvasTexture);
    SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, 255);
    switch (currentTool) {
        case TOOL_LINE:
            SDL_RenderDrawLine(renderer, startPoint.x, startPoint.y, currentPoint.x, currentPoint.y);
            break;
        case TOOL_RECTANGLE: {
            SDL_Rect rect;
            rect.x = std::min(startPoint.x, currentPoint.x);
            rect.y = std::min(startPoint.y, currentPoint.y);
            rect.w = std::abs(startPoint.x - currentPoint.x);
            rect.h = std::abs(startPoint.y - currentPoint.y);
            SDL_RenderDrawRect(renderer, &rect);
            break;
        }
        case TOOL_CIRCLE: {
            int radius = static_cast<int>(std::sqrt((currentPoint.x - startPoint.x) * (currentPoint.x - startPoint.x) +
                                                    (currentPoint.y - startPoint.y) * (currentPoint.y - startPoint.y)));
            drawCircle(renderer, startPoint.x, startPoint.y, radius);
            break;
        }
        default:
            break;
    }
    SDL_SetRenderTarget(renderer, NULL);
}

// Render a status bar at the bottom showing the current tool, brush size, and rainbow mode state
void renderStatusBar() {
    std::string toolName;
    switch (currentTool) {
        case TOOL_BRUSH: toolName = "Brush"; break;
        case TOOL_ERASER: toolName = "Eraser"; break;
        case TOOL_LINE: toolName = "Line"; break;
        case TOOL_RECTANGLE: toolName = "Rectangle"; break;
        case TOOL_CIRCLE: toolName = "Circle"; break;
        default: toolName = "Unknown"; break;
    }
    std::string status = "Tool: " + toolName + " | Brush Size: " + std::to_string(brushSize) +
                         " | Rainbow: " + (rainbowMode ? "ON" : "OFF");
    SDL_Color textColor = {0, 0, 0, 255};
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, status.c_str(), textColor);
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect destRect = {10, SCREEN_HEIGHT - textSurface->h - 10, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, NULL, &destRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}

// Clear the canvas 
void clearCanvas() {
    SDL_SetRenderTarget(renderer, canvasTexture);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, NULL);
    for (auto surface : undoStack) {
        SDL_FreeSurface(surface);
    }
    undoStack.clear();
    std::cout << "Canvas cleared.\n";
}
