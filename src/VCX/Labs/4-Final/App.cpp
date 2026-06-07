#include "Labs/4-Final/App.h"

namespace VCX::Labs::Final {
    App::App():
        _ui(Labs::Common::UIOptions {}) {
    }

    void App::OnFrame() {
        _ui.Setup(_cases, _caseId);
    }
} // namespace VCX::Labs::Final
