#  Paint Application

A feature-rich paint application built using C++ and SDL2. This project allows users to draw freehand, lines, rectangles, and circles, with a variety of colors and brush sizes. It also includes features like undo, clear canvas, save and load functionality.

## Getting Started

### Prerequisites
Ensure you have the following installed:
- C++ Compiler (e.g., g++)
- [SDL2](https://www.libsdl.org/)
- [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/)


## Project Structure

- `paint.cc`: Main C++ source file.
- `Makefile`: Build the project.
- `src/`
    - `include/`: ** Header files 
    

### Installation
1. Clone the repository:
    ```bash
    git clone git@github.com:Luke23-45/Painting-Application.git
    ```
## Building

This project uses `make` for building. To build the project, run the following command in your terminal:

1. Navigate to the project directory:
    ```bash
    cd Painting-Application
    ```
3. Compile the code:
    ```bash
     make
    ```
4. Run the executable:
    ```bash
    ./main

    ```
5. In window (if built for Windows using MinGW or similar):
    ```bash
    main.exe
    ```
6. To clean up the build artifacts
    ```bash
     make clean
    ```

## Features
- **Brush Tool**: Freehand drawing with adjustable brush size and color.
- **Eraser Tool**:  Erase parts of the canvas.
- **Line Tool**: Draw straight lines between two points.
- **Rectangle Tool**: Draw rectangles by dragging the mouse.
- **Circle Tool**: Draw circles by dragging the mouse.
- **Color Palette**: Choose from a selection of vibrant colors.
- **Rainbow Mode**: Cycle through colors automatically while drawing.
- **Adjustable Brush Size**: Increase or decrease brush size using `+` and `-` keys.
- **Save and Load Canvas**: Save your artwork as `canvas.bmp` and load it back later.
- **Undo Functionality**: Undo the last drawing action.
- **Clear Canvas**: Clear the entire canvas with a white background.
- **Status Bar**: Displays the current tool, brush size, and rainbow mode status.

## Key Controls

| Action             | Key        | Description                                  |
| ------------------ | ---------- | -------------------------------------------- |
| Exit simulation    | `ESC` key  | Quit the application                         |
| Select Brush Tool  | `1` key    | Switch to the brush tool                     |
| Select Eraser Tool | `2` key    | Switch to the eraser tool                    |
| Select Line Tool   | `3` key    | Switch to the line drawing tool              |
| Select Rectangle Tool| `4` key    | Switch to the rectangle drawing tool         |
| Select Circle Tool | `5` key    | Switch to the circle drawing tool             |
| Toggle Rainbow Mode| `M` key    | Turn rainbow color mode on or off            |
| Increase Brush Size| `+` or `KP_PLUS` key | Increase the brush size                      |
| Decrease Brush Size| `-` or `KP_MINUS` key| Decrease the brush size (minimum size is 2) |
| Save Canvas        | `S` key    | Save the current canvas to `canvas.bmp`      |
| Load Canvas        | `O` key    | Load canvas from `canvas.bmp`                |
| Undo               | `U` key    | Undo the last drawing action                 |
| Clear Canvas       | `C` key    | Clear the entire canvas                      |


## Code Structure
The project is primarily contained within `main.cc`. Here's a brief overview:
- **Initialization (`init`, `initCanvas`, `loadFont`, `initColorPalette`)**: Sets up SDL, window, renderer, canvas texture, font, and color palette.
- **Event Handling (main loop)**:  Processes SDL events for keyboard and mouse inputs to control tools, colors, and actions.
- **Drawing Tools (Brush, Eraser, Line, Rectangle, Circle)**: Implements drawing logic for each tool, including shape preview and committing shapes to the canvas.
- **Color Management (`initColorPalette`, `renderPalette`, `handleColorSelection`, `hueToRGB`)**: Handles color palette display, color selection, and rainbow color generation.
- **Canvas State Management (`saveCanvasState`, `undoLastAction`, `clearCanvas`)**: Implements undo functionality and canvas clearing by storing and restoring canvas states.
- **User Interface (`renderStatusBar`)**: Renders a status bar to display current tool and settings.
- **Helper Functions (`drawCircle`, `drawPreviewShape`, `commitShape`)**:  Provides utility functions for drawing shapes and managing shape previews.

## Demo Video
Check out the project demo video on YouTube: [Demo video to be added - Replace with your project demo video link]


## License

This project is licensed under the MIT License. Feel free to use, modify, and distribute the code.

## Acknowledgements

- SDL2 for graphics rendering.
- SDL2_ttf for font rendering.
