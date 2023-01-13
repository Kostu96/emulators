#pragma once
#include "../../shared/emulator_core.hpp"
#include "cpu6502.hpp"
#include "io.hpp"

class PETCore :
    public EmulatorCore
{
public:
    const WindowSettings& getWindowSettings() const override { return m_windowSettings; }

    void render(CharVertex* verts) override;
    void handleKey(int key, int action) override;

    void loadROM(const char* filename) override;
    void reset() override;
    void clock() override;

    PETCore();
private:
    CPU6502 m_cpu;
    IO m_io;

    const WindowSettings m_windowSettings;
};
