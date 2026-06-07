#include "Assets/bundled.h"
#include "Labs/4-Final/App.h"

int main() {
    using namespace VCX;
    return Engine::RunApp<Labs::Final::App>(Engine::AppContextOptions {
        .Title         = "VCX-sim Labs 4: Angry Rigid Birds",
        .WindowSize    = { 1280, 800 },
        .FontSize      = 16,
        .IconFileNames = Assets::DefaultIcons,
        .FontFileNames = Assets::DefaultFonts,
    });
}
