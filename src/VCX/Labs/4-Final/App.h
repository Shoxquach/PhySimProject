#pragma once

#include <functional>
#include <vector>

#include "Engine/app.h"
#include "Labs/4-Final/CaseAngryBirds3D.h"
#include "Labs/Common/UI.h"

namespace VCX::Labs::Final {
    class App : public Engine::IApp {
    private:
        Common::UI _ui;

        CaseAngryBirds3D _caseAngryBirds;

        std::size_t _caseId = 0;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = {
            _caseAngryBirds,
        };

    public:
        App();

        void OnFrame() override;
    };
} // namespace VCX::Labs::Final
