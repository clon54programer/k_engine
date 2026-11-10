#include "editor/app.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <GLFW/glfw3.h>

#include "editor/project_io.h"
#include "editor/tile_tool.h"
#include "editor/validation.h"

namespace k_engine::editor
{
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-easily-swappable-parameters,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions,modernize-use-auto,cppcoreguidelines-pro-bounds-pointer-arithmetic,bugprone-branch-clone,readability-function-cognitive-complexity)

namespace
{

constexpr float TOP_HEIGHT = 44.0F;
constexpr float LEFT_WIDTH = 260.0F;
constexpr float RIGHT_WIDTH = 300.0F;
constexpr float BOTTOM_HEIGHT = 150.0F;
constexpr float PADDING = 8.0F;
constexpr float ITEM_HEIGHT = 28.0F;
constexpr float FIELD_HEIGHT = 42.0F;
constexpr float FIELD_STEP = 46.0F;
constexpr float TEXT_SCALE = 1.05F;
constexpr float TITLE_SCALE = 1.35F;
constexpr float MIN_ZOOM = 0.25F;
constexpr float MAX_ZOOM = 4.0F;

constexpr ui::Color BACKGROUND = ui::Color::FromBytes(23, 26, 32);
constexpr ui::Color PANEL = ui::Color::FromBytes(35, 39, 47);
constexpr ui::Color CANVAS = ui::Color::FromBytes(19, 22, 27);
constexpr ui::Color TEXT = ui::Color::FromBytes(226, 231, 239);
constexpr ui::Color MUTED = ui::Color::FromBytes(150, 158, 172);
constexpr ui::Color ACCENT = ui::Color::FromBytes(64, 140, 180);
constexpr ui::Color GRID = ui::Color::FromBytes(58, 64, 76);
constexpr ui::Color ERROR_COLOR = ui::Color::FromBytes(235, 105, 105);
constexpr ui::Color COLLISION_TINT{1.0F, 0.32F, 0.32F, 0.42F};
constexpr ui::Color SELECTION_COLOR = ui::Color::FromBytes(255, 210, 90);

float Right(const ui::Rect &rect) noexcept
{
    return rect.x + rect.width;
}

float Bottom(const ui::Rect &rect) noexcept
{
    return rect.y + rect.height;
}

ui::Rect MakeRect(float x, float y, float width, float height) noexcept
{
    return ui::Rect{x, y, width, height};
}

std::string AssetLabel(const AssetRef &asset)
{
    return FormatId(asset.id) + " " +
           std::filesystem::path(asset.path).filename().string();
}

const Tileset *FindTileset(const Level &level, std::uint64_t tileset_id) noexcept
{
    const auto ITERATOR = std::find_if(
        level.tilesets.begin(), level.tilesets.end(),
        [tileset_id](const Tileset &tileset) { return tileset.id == tileset_id; });
    return ITERATOR == level.tilesets.end() ? nullptr : &(*ITERATOR);
}

const Layer *FindLayer(const Level &level, std::uint64_t layer_id) noexcept
{
    const auto ITERATOR =
        std::find_if(level.layers.begin(), level.layers.end(),
                     [layer_id](const Layer &layer) { return layer.id == layer_id; });
    return ITERATOR == level.layers.end() ? nullptr : &(*ITERATOR);
}

ui::Rect FitRect(const ui::Rect &container, std::uint32_t width, std::uint32_t height,
                 float padding, float &scale)
{
    if (width == 0 || height == 0)
    {
        scale = 1.0F;
        return container;
    }
    const float AVAILABLE_WIDTH = std::max(container.width - (2.0F * padding), 1.0F);
    const float AVAILABLE_HEIGHT = std::max(container.height - (2.0F * padding), 1.0F);
    scale = std::min(AVAILABLE_WIDTH / static_cast<float>(width),
                     AVAILABLE_HEIGHT / static_cast<float>(height));
    scale = std::max(scale, 0.01F);
    const float DRAW_WIDTH = static_cast<float>(width) * scale;
    const float DRAW_HEIGHT = static_cast<float>(height) * scale;
    return MakeRect(container.x + ((container.width - DRAW_WIDTH) / 2.0F),
                    container.y + ((container.height - DRAW_HEIGHT) / 2.0F), DRAW_WIDTH,
                    DRAW_HEIGHT);
}

ui::UvRect TileUv(const Tileset &tileset, std::uint32_t tile_index)
{
    const std::uint32_t ZERO_BASED = tile_index - 1U;
    const std::uint32_t COLUMN = ZERO_BASED % tileset.columns;
    const std::uint32_t ROW = ZERO_BASED / tileset.columns;
    return ui::UvRect{static_cast<float>(COLUMN) / static_cast<float>(tileset.columns),
                      static_cast<float>(ROW) / static_cast<float>(tileset.rows),
                      static_cast<float>(COLUMN + 1U) /
                          static_cast<float>(tileset.columns),
                      static_cast<float>(ROW + 1U) / static_cast<float>(tileset.rows)};
}

ui::UvRect PixelUv(const Frame &frame, std::uint32_t image_width,
                   std::uint32_t image_height)
{
    return ui::UvRect{
        static_cast<float>(frame.x) / static_cast<float>(image_width),
        static_cast<float>(frame.y) / static_cast<float>(image_height),
        static_cast<float>(frame.x + frame.width) / static_cast<float>(image_width),
        static_cast<float>(frame.y + frame.height) / static_cast<float>(image_height)};
}

bool SameFrame(const Frame &left, const Frame &right) noexcept
{
    return left.x == right.x && left.y == right.y && left.width == right.width &&
           left.height == right.height;
}

} // namespace

EditorApp::EditorApp() = default;

EditorApp::~EditorApp()
{
    Shutdown();
}

bool EditorApp::Initialize(const EditorOptions &options)
{
    _options = options;
    _shutdown_complete = false;

    gfx::ContextOptions context_options;
    context_options.title = "K_engine - Editor T3";
    context_options.width = 1280;
    context_options.height = 720;
    if (!_context.Initialize(context_options))
    {
        return false;
    }

    install_callbacks();

    if (!_renderer.Initialize(_context, _context.RenderPass(), _context.Extent()))
    {
        std::cerr << "Error: no se pudo crear el renderer del editor.\n";
        return false;
    }
    _render_generation = _context.SwapchainGeneration();
    _textures.Initialize(&_renderer);

    if (!options.new_directory.empty())
    {
        std::string error;
        if (!ProjectIo::Create(options.new_directory, _bundle, error))
        {
            std::cerr << "Error: " << error << '\n';
            return false;
        }
        _has_project = true;
        _mode = Mode::LEVEL;
        set_status("Proyecto nuevo. Importa un PNG y crea un nivel.", false);
    }
    else if (!options.project_path.empty())
    {
        std::string error;
        if (!ProjectIo::Load(options.project_path, _bundle, error))
        {
            std::cerr << "Error: " << error << '\n';
            return false;
        }
        _has_project = true;
        set_status("Proyecto cargado.", false);
        ensure_textures();
    }
    else
    {
        set_status("Sin proyecto. Usa --new o --project.", false);
    }

    refresh_selection();
    if (!options.import_png.empty() && _has_project)
    {
        import_png(options.import_png);
    }
    ensure_textures();

    _last_frame_time = glfwGetTime();
    return true;
}

int EditorApp::Run(const EditorOptions &options)
{
    if (_context.Window() == nullptr || _context.Device() == VK_NULL_HANDLE)
    {
        std::cerr << "Error: el editor no está inicializado.\n";
        return 1;
    }

    std::uint32_t frames_presented = 0;
    while (glfwWindowShouldClose(_context.Window()) == GLFW_FALSE &&
           (options.frame_limit == 0 || frames_presented < options.frame_limit))
    {
        glfwPollEvents();

        if (_save_requested)
        {
            _save_requested = false;
            save_project();
        }

        if (glfwWindowShouldClose(_context.Window()) != GLFW_FALSE)
        {
            if (_has_project && _bundle.dirty)
            {
                std::string error;
                if (!ProjectIo::Save(_bundle, error))
                {
                    set_status(error, true);
                    glfwSetWindowShouldClose(_context.Window(), GLFW_FALSE);
                }
            }
            if (glfwWindowShouldClose(_context.Window()) != GLFW_FALSE)
            {
                break;
            }
        }

        const double NOW = glfwGetTime();
        double delta = NOW - _last_frame_time;
        _last_frame_time = NOW;
        delta = std::clamp(delta, 0.0, 0.1);

        if (_mode == Mode::ANIMATION && _playing)
        {
            AnimationDocument *const ANIMATION = current_animation();
            if (ANIMATION != nullptr)
            {
                _player.Update(ANIMATION->data, delta);
            }
        }

        if (!_context.BeginFrame())
        {
            reset_transient_input();
            continue;
        }

        if (_context.SwapchainGeneration() != _render_generation)
        {
            if (!_renderer.RecreatePipeline(_context.RenderPass(), _context.Extent()))
            {
                std::cerr << "Error: no se pudo recrear el pipeline del editor.\n";
                return 1;
            }
            _render_generation = _context.SwapchainGeneration();
        }

        const VkExtent2D EXTENT = _context.Extent();
        _builder.BeginFrame(MakeRect(0.0F, 0.0F, static_cast<float>(EXTENT.width),
                                     static_cast<float>(EXTENT.height)),
                            _input);
        build_ui();
        _builder.EndFrame();

        if (!_renderer.Draw(_context.CommandBuffer(), _builder.GetDrawList()))
        {
            std::cerr << "Error: falló el dibujo del editor.\n";
            return 1;
        }
        if (!_context.EndFrame())
        {
            std::cerr << "Error: falló la presentación del editor.\n";
            return 1;
        }

        ++frames_presented;
        _previous_mouse = _input.mouse;
        reset_transient_input();
    }

    _context.WaitIdle();
    if (_context.ValidationEnabled() &&
        (_context.EngineErrorCount() > 0 || _context.EngineWarningCount() > 0))
    {
        std::cerr << "Error: la validación registró mensajes del engine.\n";
        return 1;
    }
    return 0;
}

void EditorApp::Shutdown() noexcept
{
    if (_shutdown_complete)
    {
        return;
    }
    _textures.Clear();
    _renderer.Shutdown();
    _context.Shutdown();
    _shutdown_complete = true;
}

void EditorApp::install_callbacks()
{
    GLFWwindow *const WINDOW = _context.Window();
    glfwSetWindowUserPointer(WINDOW, this);
    glfwSetFramebufferSizeCallback(WINDOW, &EditorApp::framebuffer_size_callback);
    glfwSetKeyCallback(WINDOW, &EditorApp::key_callback);
    glfwSetCharCallback(WINDOW, &EditorApp::char_callback);
    glfwSetCursorPosCallback(WINDOW, &EditorApp::cursor_position_callback);
    glfwSetMouseButtonCallback(WINDOW, &EditorApp::mouse_button_callback);
    glfwSetScrollCallback(WINDOW, &EditorApp::scroll_callback);
    glfwSetDropCallback(WINDOW, &EditorApp::drop_callback);
}

void EditorApp::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void)width;
    (void)height;
    auto *const APP = static_cast<EditorApp *>(glfwGetWindowUserPointer(window));
    if (APP != nullptr)
    {
        APP->_context.MarkFramebufferResized();
    }
}

void EditorApp::key_callback(GLFWwindow *window, int key, int scancode, int action,
                             int mods)
{
    (void)scancode;
    auto *const APP = static_cast<EditorApp *>(glfwGetWindowUserPointer(window));
    if (APP == nullptr)
    {
        return;
    }
    APP->_ctrl_pressed = (mods & GLFW_MOD_CONTROL) != 0;
    APP->_shift_pressed = (mods & GLFW_MOD_SHIFT) != 0;
    APP->_input.ctrl = APP->_ctrl_pressed;
    APP->_input.shift = APP->_shift_pressed;

    if (action != GLFW_PRESS && action != GLFW_REPEAT)
    {
        return;
    }
    if (key == GLFW_KEY_ESCAPE)
    {
        APP->_input.escape = true;
    }
    else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER)
    {
        APP->_input.enter = true;
    }
    else if (key == GLFW_KEY_BACKSPACE)
    {
        APP->_input.backspace = true;
    }
    else if (key == GLFW_KEY_S && APP->_ctrl_pressed)
    {
        APP->_save_requested = true;
    }
}

void EditorApp::char_callback(GLFWwindow *window, unsigned int codepoint)
{
    auto *const APP = static_cast<EditorApp *>(glfwGetWindowUserPointer(window));
    if (APP == nullptr || APP->_ctrl_pressed)
    {
        return;
    }
    if (codepoint >= 32U && codepoint <= 126U)
    {
        APP->_input.text.push_back(static_cast<char>(codepoint));
    }
}

void EditorApp::cursor_position_callback(GLFWwindow *window, double cursor_x,
                                         double cursor_y)
{
    auto *const APP = static_cast<EditorApp *>(glfwGetWindowUserPointer(window));
    if (APP != nullptr)
    {
        APP->_input.mouse =
            ui::Vec2{static_cast<float>(cursor_x), static_cast<float>(cursor_y)};
    }
}

void EditorApp::mouse_button_callback(GLFWwindow *window, int button, int action,
                                      int mods)
{
    (void)mods;
    auto *const APP = static_cast<EditorApp *>(glfwGetWindowUserPointer(window));
    if (APP == nullptr)
    {
        return;
    }

    const bool PRESSED = action == GLFW_PRESS;
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        APP->_input.left_down = PRESSED;
        APP->_input.left_pressed = PRESSED;
        APP->_input.left_released = !PRESSED;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        APP->_input.right_down = PRESSED;
        APP->_input.right_pressed = PRESSED;
        APP->_input.right_released = !PRESSED;
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        APP->_input.middle_down = PRESSED;
    }
}

void EditorApp::scroll_callback(GLFWwindow *window, double offset_x, double offset_y)
{
    (void)offset_x;
    auto *const APP = static_cast<EditorApp *>(glfwGetWindowUserPointer(window));
    if (APP != nullptr)
    {
        APP->_input.wheel += static_cast<float>(offset_y);
    }
}

void EditorApp::drop_callback(GLFWwindow *window, int count, const char **paths)
{
    auto *const APP = static_cast<EditorApp *>(glfwGetWindowUserPointer(window));
    if (APP == nullptr)
    {
        return;
    }
    for (int index = 0; index < count; ++index)
    {
        if (paths[index] != nullptr)
        {
            APP->import_png(paths[index]);
        }
    }
}

void EditorApp::reset_transient_input() noexcept
{
    _input.text.clear();
    _input.left_pressed = false;
    _input.left_released = false;
    _input.right_pressed = false;
    _input.right_released = false;
    _input.backspace = false;
    _input.enter = false;
    _input.escape = false;
    _input.wheel = 0.0F;
}

LevelDocument *EditorApp::current_level() noexcept
{
    if (_selected_level < 0 ||
        static_cast<std::size_t>(_selected_level) >= _bundle.levels.size())
    {
        return nullptr;
    }
    return &_bundle.levels[static_cast<std::size_t>(_selected_level)];
}

const LevelDocument *EditorApp::current_level() const noexcept
{
    if (_selected_level < 0 ||
        static_cast<std::size_t>(_selected_level) >= _bundle.levels.size())
    {
        return nullptr;
    }
    return &_bundle.levels[static_cast<std::size_t>(_selected_level)];
}

AnimationDocument *EditorApp::current_animation() noexcept
{
    if (_selected_animation < 0 ||
        static_cast<std::size_t>(_selected_animation) >= _bundle.animations.size())
    {
        return nullptr;
    }
    return &_bundle.animations[static_cast<std::size_t>(_selected_animation)];
}

const AnimationDocument *EditorApp::current_animation() const noexcept
{
    if (_selected_animation < 0 ||
        static_cast<std::size_t>(_selected_animation) >= _bundle.animations.size())
    {
        return nullptr;
    }
    return &_bundle.animations[static_cast<std::size_t>(_selected_animation)];
}

const gfx::TextureCache::Entry *EditorApp::texture_for(std::uint64_t asset_id)
{
    if (const auto *const EXISTING = _textures.Find(asset_id); EXISTING != nullptr)
    {
        return EXISTING;
    }
    if (_failed_textures.count(asset_id) != 0)
    {
        return nullptr;
    }
    const AssetRef *const ASSET = FindAsset(_bundle, asset_id);
    if (ASSET == nullptr)
    {
        return nullptr;
    }

    std::string error;
    const auto *const ENTRY =
        _textures.Load(asset_id, _bundle.base_directory / ASSET->path, error);
    if (ENTRY == nullptr)
    {
        _failed_textures.insert(asset_id);
        set_status(error, true);
    }
    return ENTRY;
}

void EditorApp::ensure_textures()
{
    for (const AssetRef &ASSET : _bundle.project.assets)
    {
        (void)texture_for(ASSET.id);
    }
}

void EditorApp::refresh_selection() noexcept
{
    if (_bundle.levels.empty())
    {
        _selected_level = -1;
    }
    else if (_selected_level < 0 ||
             static_cast<std::size_t>(_selected_level) >= _bundle.levels.size())
    {
        _selected_level = 0;
    }

    if (_bundle.animations.empty())
    {
        _selected_animation = -1;
    }
    else if (_selected_animation < 0 ||
             static_cast<std::size_t>(_selected_animation) >= _bundle.animations.size())
    {
        _selected_animation = 0;
    }

    if (_bundle.project.assets.empty())
    {
        _selected_asset = -1;
    }
    else if (_selected_asset < 0 ||
             static_cast<std::size_t>(_selected_asset) >= _bundle.project.assets.size())
    {
        _selected_asset = 0;
    }

    const LevelDocument *const LEVEL = current_level();
    if (LEVEL == nullptr || LEVEL->data.layers.empty())
    {
        _selected_layer = 0;
    }
    else if (_selected_layer < 0 ||
             static_cast<std::size_t>(_selected_layer) >= LEVEL->data.layers.size())
    {
        _selected_layer = 0;
    }
}

void EditorApp::set_status(std::string message, bool error)
{
    _status = std::move(message);
    _status_error = error;
    if (error)
    {
        std::cerr << "Error: " << _status << '\n';
    }
}

void EditorApp::mark_dirty() noexcept
{
    if (_has_project)
    {
        _bundle.dirty = true;
    }
}

void EditorApp::save_project()
{
    if (!_has_project)
    {
        set_status("No hay un proyecto abierto.", true);
        return;
    }
    std::string error;
    if (!ProjectIo::Save(_bundle, error))
    {
        set_status(error, true);
        return;
    }
    set_status("Proyecto guardado.", false);
    std::cout << "Proyecto guardado en " << _bundle.project_path.string() << ".\n";
}

void EditorApp::import_png(const std::filesystem::path &path)
{
    if (!_has_project)
    {
        set_status("Abre o crea un proyecto antes de importar un PNG.", true);
        return;
    }
    std::uint64_t asset_id = 0;
    std::string error;
    if (!ProjectIo::ImportPng(_bundle, path, asset_id, error))
    {
        set_status(error, true);
        return;
    }
    _failed_textures.erase(asset_id);
    refresh_selection();
    _selected_asset = static_cast<int>(_bundle.project.assets.size()) - 1;
    (void)texture_for(asset_id);
    set_status("PNG importado como asset " + FormatId(asset_id) + ".", false);
    std::cout << "PNG importado como asset " << asset_id << ".\n";
}

void EditorApp::create_level()
{
    if (!_has_project)
    {
        set_status("No hay un proyecto abierto.", true);
        return;
    }
    if (_selected_asset < 0 ||
        static_cast<std::size_t>(_selected_asset) >= _bundle.project.assets.size())
    {
        set_status("Selecciona o importa un PNG antes de crear un nivel.", true);
        return;
    }
    const std::uint64_t ASSET_ID =
        _bundle.project.assets[static_cast<std::size_t>(_selected_asset)].id;
    std::uint64_t level_id = 0;
    std::string error;
    if (!ProjectIo::CreateLevel(_bundle, ASSET_ID, level_id, error))
    {
        set_status(error, true);
        return;
    }
    _selected_level = static_cast<int>(_bundle.levels.size()) - 1;
    _selected_layer = 0;
    _mode = Mode::LEVEL;
    _zoom = 1.0;
    _pan_x = 0.0;
    _pan_y = 0.0;
    _selected_tile = 1;
    set_status("Nivel " + FormatId(level_id) + " creado.", false);
    std::cout << "Nivel " << level_id << " creado.\n";
}

void EditorApp::create_animation()
{
    if (!_has_project)
    {
        set_status("No hay un proyecto abierto.", true);
        return;
    }
    if (_selected_asset < 0 ||
        static_cast<std::size_t>(_selected_asset) >= _bundle.project.assets.size())
    {
        set_status("Selecciona o importa un PNG antes de crear una animación.", true);
        return;
    }
    const std::uint64_t ASSET_ID =
        _bundle.project.assets[static_cast<std::size_t>(_selected_asset)].id;
    std::uint64_t animation_id = 0;
    std::string error;
    if (!ProjectIo::CreateAnimation(_bundle, ASSET_ID, animation_id, error))
    {
        set_status(error, true);
        return;
    }
    _selected_animation = static_cast<int>(_bundle.animations.size()) - 1;
    _mode = Mode::ANIMATION;
    _player.Reset();
    set_status("Animación " + FormatId(animation_id) + " creada.", false);
    std::cout << "Animacion " << animation_id << " creada.\n";
}

void EditorApp::add_layer(LayerKind kind)
{
    LevelDocument *const LEVEL = current_level();
    if (LEVEL == nullptr)
    {
        set_status("Selecciona un nivel.", true);
        return;
    }
    if (LEVEL->data.layers.size() >= MAX_LAYERS)
    {
        set_status("El nivel ya tiene 16 capas.", true);
        return;
    }
    if (LEVEL->data.tilesets.empty())
    {
        set_status("El nivel no tiene ningún tileset.", true);
        return;
    }
    std::uint64_t tileset_id = LEVEL->data.tilesets.front().id;
    if (_selected_tileset >= 0 &&
        static_cast<std::size_t>(_selected_tileset) < LEVEL->data.tilesets.size())
    {
        tileset_id = LEVEL->data.tilesets[static_cast<std::size_t>(_selected_tileset)].id;
    }

    std::string error;
    const std::uint64_t NEW_ID = AllocateLocalId(LEVEL->data, error);
    if (NEW_ID == 0)
    {
        set_status(error, true);
        return;
    }

    Layer layer;
    layer.id = NEW_ID;
    layer.name = kind == LayerKind::COLLISION ? "colision" : "capa";
    layer.kind = kind;
    layer.visible = kind == LayerKind::TILE;
    layer.tileset_id = tileset_id;
    layer.tiles.assign(
        static_cast<std::size_t>(LEVEL->data.grid.columns) * LEVEL->data.grid.rows, 0);
    LEVEL->data.layers.push_back(std::move(layer));
    _selected_layer = static_cast<int>(LEVEL->data.layers.size()) - 1;
    mark_dirty();
    set_status("Capa añadida.", false);
}

void EditorApp::delete_selected_layer()
{
    LevelDocument *const LEVEL = current_level();
    if (LEVEL == nullptr || LEVEL->data.layers.size() <= 1)
    {
        set_status("El nivel debe conservar al menos una capa.", true);
        return;
    }
    if (_selected_layer < 0 ||
        static_cast<std::size_t>(_selected_layer) >= LEVEL->data.layers.size())
    {
        return;
    }
    LEVEL->data.layers.erase(LEVEL->data.layers.begin() + _selected_layer);
    _selected_layer =
        std::min<int>(_selected_layer, static_cast<int>(LEVEL->data.layers.size()) - 1);
    mark_dirty();
    set_status("Capa eliminada.", false);
}

void EditorApp::add_tileset()
{
    LevelDocument *const LEVEL = current_level();
    if (LEVEL == nullptr)
    {
        set_status("Selecciona un nivel.", true);
        return;
    }
    if (_selected_asset < 0 ||
        static_cast<std::size_t>(_selected_asset) >= _bundle.project.assets.size())
    {
        set_status("Selecciona un PNG para crear el tileset.", true);
        return;
    }
    const std::uint64_t ASSET_ID =
        _bundle.project.assets[static_cast<std::size_t>(_selected_asset)].id;
    std::uint64_t tileset_id = 0;
    std::string error;
    if (!ProjectIo::AddTileset(_bundle, LEVEL->data, ASSET_ID, tileset_id, error))
    {
        set_status(error, true);
        return;
    }
    _selected_tileset = static_cast<int>(LEVEL->data.tilesets.size()) - 1;
    set_status("Tileset " + FormatId(tileset_id) + " añadido.", false);
}

void EditorApp::set_grid_tile_size(std::uint32_t tile_width, std::uint32_t tile_height)
{
    LevelDocument *const LEVEL = current_level();
    if (LEVEL == nullptr || tile_width == 0 || tile_height == 0 ||
        tile_width > MAX_GRID_DIMENSION || tile_height > MAX_GRID_DIMENSION)
    {
        return;
    }

    for (const Tileset &TILESET : LEVEL->data.tilesets)
    {
        const auto *const ENTRY = texture_for(TILESET.image_id);
        if (ENTRY == nullptr)
        {
            set_status("No se pudo leer la imagen de un tileset.", true);
            return;
        }
        if (ENTRY->width / tile_width == 0 || ENTRY->height / tile_height == 0)
        {
            set_status("El nuevo tamaño de tile no cabe en la imagen.", true);
            return;
        }
    }

    LEVEL->data.grid.tile_width = tile_width;
    LEVEL->data.grid.tile_height = tile_height;
    for (Tileset &tileset : LEVEL->data.tilesets)
    {
        const auto *const ENTRY = texture_for(tileset.image_id);
        if (ENTRY == nullptr)
        {
            continue;
        }
        tileset.tile_width = tile_width;
        tileset.tile_height = tile_height;
        tileset.columns = ENTRY->width / tile_width;
        tileset.rows = ENTRY->height / tile_height;
        const std::uint64_t TILE_LIMIT =
            static_cast<std::uint64_t>(tileset.columns) * tileset.rows;
        for (Layer &layer : LEVEL->data.layers)
        {
            if (layer.tileset_id != tileset.id)
            {
                continue;
            }
            for (std::uint32_t &tile : layer.tiles)
            {
                if (static_cast<std::uint64_t>(tile) > TILE_LIMIT)
                {
                    tile = 0;
                }
            }
        }
    }
    mark_dirty();
}

void EditorApp::set_grid_dimensions(std::uint32_t columns, std::uint32_t rows)
{
    LevelDocument *const LEVEL = current_level();
    if (LEVEL == nullptr)
    {
        return;
    }
    if (!ResizeGrid(LEVEL->data, columns, rows))
    {
        set_status("Dimensiones de cuadrícula fuera de límite.", true);
        return;
    }
    mark_dirty();
}

void EditorApp::build_ui()
{
    const VkExtent2D EXTENT = _context.Extent();
    const float WIDTH = static_cast<float>(EXTENT.width);
    const float HEIGHT = static_cast<float>(EXTENT.height);
    _context.SetClearColor(BACKGROUND.r, BACKGROUND.g, BACKGROUND.b, BACKGROUND.a);

    const ui::Rect TOP = MakeRect(0.0F, 0.0F, WIDTH, TOP_HEIGHT);
    const ui::Rect LEFT = MakeRect(0.0F, TOP_HEIGHT, LEFT_WIDTH, HEIGHT - TOP_HEIGHT);
    const ui::Rect RIGHT =
        MakeRect(WIDTH - RIGHT_WIDTH, TOP_HEIGHT, RIGHT_WIDTH, HEIGHT - TOP_HEIGHT);
    const float CENTER_WIDTH = std::max(WIDTH - LEFT_WIDTH - RIGHT_WIDTH, 1.0F);
    const float CENTER_HEIGHT = std::max(HEIGHT - TOP_HEIGHT - BOTTOM_HEIGHT, 1.0F);
    const ui::Rect CENTER = MakeRect(LEFT_WIDTH, TOP_HEIGHT, CENTER_WIDTH, CENTER_HEIGHT);
    const ui::Rect BOTTOM =
        MakeRect(LEFT_WIDTH, HEIGHT - BOTTOM_HEIGHT, CENTER_WIDTH, BOTTOM_HEIGHT);

    build_top_bar(TOP);
    build_left_panel(LEFT);
    build_right_panel(RIGHT);
    if (_mode == Mode::LEVEL)
    {
        build_level_canvas(CENTER);
        build_level_palette(BOTTOM);
    }
    else
    {
        build_animation_canvas(CENTER);
        build_animation_sheet(BOTTOM);
    }
}

void EditorApp::build_top_bar(const ui::Rect &rect)
{
    _builder.Panel(ui::MakeId("top"), rect, PANEL, GRID);
    _builder.Text(ui::MakeId("title"),
                  MakeRect(rect.x + 10.0F, rect.y, 210.0F, rect.height),
                  "K_engine - Editor T3", TITLE_SCALE, TEXT);

    float x = 220.0F;
    if (_builder.Button(ui::MakeId("save"), MakeRect(x, rect.y + 8.0F, 80.0F, 28.0F),
                        "Guardar"))
    {
        save_project();
    }
    x += 88.0F;

    if (_builder.Button(ui::MakeId("mode_level"),
                        MakeRect(x, rect.y + 8.0F, 90.0F, 28.0F), "Nivel",
                        _mode == Mode::LEVEL))
    {
        _mode = Mode::LEVEL;
    }
    x += 96.0F;
    if (_builder.Button(ui::MakeId("mode_animation"),
                        MakeRect(x, rect.y + 8.0F, 110.0F, 28.0F), "Animacion",
                        _mode == Mode::ANIMATION))
    {
        _mode = Mode::ANIMATION;
    }
    x += 118.0F;

    if (_builder.Button(ui::MakeId("new_level"),
                        MakeRect(x, rect.y + 8.0F, 100.0F, 28.0F), "Nuevo nivel"))
    {
        create_level();
    }
    x += 108.0F;
    if (_builder.Button(ui::MakeId("new_animation"),
                        MakeRect(x, rect.y + 8.0F, 130.0F, 28.0F), "Nueva animacion"))
    {
        create_animation();
    }

    const float STATUS_X = std::min(x + 145.0F, rect.width - 360.0F);
    if (STATUS_X > 0.0F)
    {
        _builder.Text(
            ui::MakeId("status"),
            MakeRect(STATUS_X, rect.y, rect.width - STATUS_X - 10.0F, rect.height),
            _status.empty() ? "Ctrl+S guardar" : _status, TEXT_SCALE,
            _status_error ? ERROR_COLOR : MUTED, ui::TextAlign::RIGHT);
    }
}

void EditorApp::build_left_panel(const ui::Rect &rect)
{
    _builder.Panel(ui::MakeId("left"), rect, PANEL, GRID);
    const float X = rect.x + PADDING;
    const float WIDTH = rect.width - (2.0F * PADDING);
    float y = rect.y + PADDING;

    if (!_has_project)
    {
        _builder.Text(ui::MakeId("no_project"), MakeRect(X, y, WIDTH, 40.0F),
                      "Sin proyecto. Usa --new o --project.", TEXT_SCALE, MUTED);
        return;
    }

    if (_builder.TextField(ui::MakeId("project_name"),
                           MakeRect(X, y, WIDTH, FIELD_HEIGHT), "Proyecto",
                           _bundle.project.name))
    {
        mark_dirty();
    }
    y += FIELD_HEIGHT + 8.0F;

    _builder.Text(ui::MakeId("assets_title"), MakeRect(X, y, WIDTH, 18.0F), "Assets",
                  TEXT_SCALE, MUTED);
    y += 20.0F;
    for (std::size_t index = 0; index < _bundle.project.assets.size(); ++index)
    {
        if (y + ITEM_HEIGHT > Bottom(rect) - 240.0F)
        {
            break;
        }
        const AssetRef &ASSET = _bundle.project.assets[index];
        if (_builder.Button(ui::MakeId("asset_" + FormatId(ASSET.id)),
                            MakeRect(X, y, WIDTH, ITEM_HEIGHT), AssetLabel(ASSET),
                            static_cast<int>(index) == _selected_asset))
        {
            _selected_asset = static_cast<int>(index);
        }
        y += ITEM_HEIGHT + 2.0F;
    }
    if (_bundle.project.assets.empty())
    {
        _builder.Text(ui::MakeId("assets_hint"), MakeRect(X, y, WIDTH, 18.0F),
                      "Arrastra un PNG aqui.", TEXT_SCALE, MUTED);
        y += 20.0F;
    }
    y += 8.0F;

    _builder.Text(ui::MakeId("levels_title"), MakeRect(X, y, WIDTH, 18.0F), "Niveles",
                  TEXT_SCALE, MUTED);
    y += 20.0F;
    for (std::size_t index = 0; index < _bundle.levels.size(); ++index)
    {
        if (y + ITEM_HEIGHT > Bottom(rect) - 120.0F)
        {
            break;
        }
        const Level &LEVEL = _bundle.levels[index].data;
        if (_builder.Button(
                ui::MakeId("level_" + FormatId(LEVEL.id)),
                MakeRect(X, y, WIDTH, ITEM_HEIGHT), FormatId(LEVEL.id) + " " + LEVEL.name,
                static_cast<int>(index) == _selected_level && _mode == Mode::LEVEL))
        {
            _selected_level = static_cast<int>(index);
            _selected_layer = 0;
            _mode = Mode::LEVEL;
            _zoom = 1.0;
            _pan_x = 0.0;
            _pan_y = 0.0;
        }
        y += ITEM_HEIGHT + 2.0F;
    }
    y += 8.0F;

    _builder.Text(ui::MakeId("animations_title"), MakeRect(X, y, WIDTH, 18.0F),
                  "Animaciones", TEXT_SCALE, MUTED);
    y += 20.0F;
    for (std::size_t index = 0; index < _bundle.animations.size(); ++index)
    {
        if (y + ITEM_HEIGHT > Bottom(rect) - 20.0F)
        {
            break;
        }
        const Animation &ANIMATION = _bundle.animations[index].data;
        if (_builder.Button(ui::MakeId("animation_" + FormatId(ANIMATION.id)),
                            MakeRect(X, y, WIDTH, ITEM_HEIGHT),
                            FormatId(ANIMATION.id) + " " + ANIMATION.name,
                            static_cast<int>(index) == _selected_animation &&
                                _mode == Mode::ANIMATION))
        {
            _selected_animation = static_cast<int>(index);
            _mode = Mode::ANIMATION;
            _player.Reset();
        }
        y += ITEM_HEIGHT + 2.0F;
    }
}

void EditorApp::build_right_panel(const ui::Rect &rect)
{
    _builder.Panel(ui::MakeId("right"), rect, PANEL, GRID);
    if (_mode == Mode::LEVEL)
    {
        build_level_properties(rect);
    }
    else
    {
        build_animation_properties(rect);
    }
}

void EditorApp::build_level_properties(const ui::Rect &rect)
{
    const float X = rect.x + PADDING;
    const float WIDTH = rect.width - (2.0F * PADDING);
    float y = rect.y + PADDING;

    LevelDocument *const LEVEL_DOCUMENT = current_level();
    if (LEVEL_DOCUMENT == nullptr)
    {
        _builder.Text(ui::MakeId("no_level"), MakeRect(X, y, WIDTH, 60.0F),
                      "Crea o selecciona un nivel.", TEXT_SCALE, MUTED);
        return;
    }
    Level &level = LEVEL_DOCUMENT->data;

    _builder.Text(ui::MakeId("level_title"), MakeRect(X, y, WIDTH, 20.0F),
                  "Nivel " + FormatId(level.id), TITLE_SCALE, TEXT);
    y += 24.0F;
    if (_builder.TextField(ui::MakeId("level_name"), MakeRect(X, y, WIDTH, FIELD_HEIGHT),
                           "Nombre", level.name))
    {
        mark_dirty();
    }
    y += FIELD_STEP;

    int tile_width = static_cast<int>(level.grid.tile_width);
    if (_builder.NumberField(ui::MakeId("grid_tile_w"),
                             MakeRect(X, y, WIDTH, FIELD_HEIGHT), "Tile width",
                             tile_width, 1, 4096))
    {
        set_grid_tile_size(static_cast<std::uint32_t>(tile_width),
                           level.grid.tile_height);
    }
    y += FIELD_STEP;
    int tile_height = static_cast<int>(level.grid.tile_height);
    if (_builder.NumberField(ui::MakeId("grid_tile_h"),
                             MakeRect(X, y, WIDTH, FIELD_HEIGHT), "Tile height",
                             tile_height, 1, 4096))
    {
        set_grid_tile_size(level.grid.tile_width,
                           static_cast<std::uint32_t>(tile_height));
    }
    y += FIELD_STEP;

    int columns = static_cast<int>(level.grid.columns);
    if (_builder.NumberField(ui::MakeId("grid_columns"),
                             MakeRect(X, y, WIDTH, FIELD_HEIGHT), "Columnas", columns, 1,
                             4096))
    {
        set_grid_dimensions(static_cast<std::uint32_t>(columns), level.grid.rows);
    }
    y += FIELD_STEP;
    int rows = static_cast<int>(level.grid.rows);
    if (_builder.NumberField(ui::MakeId("grid_rows"), MakeRect(X, y, WIDTH, FIELD_HEIGHT),
                             "Filas", rows, 1, 4096))
    {
        set_grid_dimensions(level.grid.columns, static_cast<std::uint32_t>(rows));
    }
    y += FIELD_STEP;

    if (_builder.Button(ui::MakeId("entry_level"), MakeRect(X, y, WIDTH, 28.0F),
                        "Marcar como nivel de entrada",
                        _bundle.project.entry_level_id == level.id))
    {
        _bundle.project.entry_level_id = level.id;
        mark_dirty();
    }
    y += 36.0F;

    _builder.Text(ui::MakeId("layers_title"), MakeRect(X, y, WIDTH, 18.0F), "Capas",
                  TEXT_SCALE, MUTED);
    y += 20.0F;
    for (std::size_t index = 0; index < level.layers.size(); ++index)
    {
        if (y + ITEM_HEIGHT > Bottom(rect) - 150.0F)
        {
            break;
        }
        const Layer &LAYER = level.layers[index];
        if (_builder.Button(ui::MakeId("layer_" + FormatId(LAYER.id)),
                            MakeRect(X, y, WIDTH, ITEM_HEIGHT),
                            (LAYER.kind == LayerKind::COLLISION ? "[C] " : "[T] ") +
                                LAYER.name,
                            static_cast<int>(index) == _selected_layer))
        {
            _selected_layer = static_cast<int>(index);
            std::cout << "Capa seleccionada: " << _selected_layer << ".\n";
        }
        y += ITEM_HEIGHT + 2.0F;
    }

    if (y + 30.0F < Bottom(rect) - 130.0F)
    {
        if (_builder.Button(ui::MakeId("add_tile_layer"),
                            MakeRect(X, y, (WIDTH - 4.0F) / 2.0F, 26.0F), "Capa tile"))
        {
            add_layer(LayerKind::TILE);
        }
        if (_builder.Button(
                ui::MakeId("add_collision_layer"),
                MakeRect(X + ((WIDTH + 4.0F) / 2.0F), y, (WIDTH - 4.0F) / 2.0F, 26.0F),
                "Capa colision"))
        {
            add_layer(LayerKind::COLLISION);
        }
        y += 32.0F;
        if (_builder.Button(ui::MakeId("delete_layer"), MakeRect(X, y, WIDTH, 26.0F),
                            "Eliminar capa"))
        {
            delete_selected_layer();
        }
        y += 32.0F;
    }

    if (_selected_layer >= 0 &&
        static_cast<std::size_t>(_selected_layer) < level.layers.size())
    {
        Layer &layer = level.layers[static_cast<std::size_t>(_selected_layer)];
        if (y + FIELD_HEIGHT < Bottom(rect) - 80.0F)
        {
            if (_builder.TextField(ui::MakeId("layer_name"),
                                   MakeRect(X, y, WIDTH, FIELD_HEIGHT), "Nombre de capa",
                                   layer.name))
            {
                mark_dirty();
            }
            y += FIELD_STEP;
        }
        if (_builder.Toggle(ui::MakeId("layer_visible"), MakeRect(X, y, WIDTH, 26.0F),
                            "Visible", layer.visible))
        {
            mark_dirty();
        }
        y += 32.0F;

        const Tileset *const TILESET = FindTileset(level, layer.tileset_id);
        if (TILESET != nullptr)
        {
            _builder.Text(ui::MakeId("tileset_info"), MakeRect(X, y, WIDTH, 18.0F),
                          "Tileset " + FormatId(TILESET->id) + " " +
                              std::to_string(TILESET->columns) + "x" +
                              std::to_string(TILESET->rows),
                          TEXT_SCALE, MUTED);
        }
        y += 22.0F;
        if (y + 26.0F < Bottom(rect) - 20.0F &&
            _builder.Button(ui::MakeId("add_tileset"), MakeRect(X, y, WIDTH, 26.0F),
                            "Anadir tileset"))
        {
            add_tileset();
        }
    }
}

void EditorApp::build_animation_properties(const ui::Rect &rect)
{
    const float X = rect.x + PADDING;
    const float WIDTH = rect.width - (2.0F * PADDING);
    float y = rect.y + PADDING;

    AnimationDocument *const ANIMATION_DOCUMENT = current_animation();
    if (ANIMATION_DOCUMENT == nullptr)
    {
        _builder.Text(ui::MakeId("no_animation"), MakeRect(X, y, WIDTH, 60.0F),
                      "Crea o selecciona una animación.", TEXT_SCALE, MUTED);
        return;
    }
    Animation &animation = ANIMATION_DOCUMENT->data;

    _builder.Text(ui::MakeId("animation_title"), MakeRect(X, y, WIDTH, 20.0F),
                  "Animacion " + FormatId(animation.id), TITLE_SCALE, TEXT);
    y += 24.0F;
    if (_builder.TextField(ui::MakeId("animation_name"),
                           MakeRect(X, y, WIDTH, FIELD_HEIGHT), "Nombre", animation.name))
    {
        mark_dirty();
    }
    y += FIELD_STEP;

    _builder.Text(ui::MakeId("animation_image"), MakeRect(X, y, WIDTH, 18.0F),
                  "Imagen " + FormatId(animation.image_id), TEXT_SCALE, MUTED);
    y += 22.0F;

    int frame_width = static_cast<int>(animation.frame_width);
    int frame_height = static_cast<int>(animation.frame_height);
    const bool WIDTH_CHANGED = _builder.NumberField(ui::MakeId("frame_width"),
                                                    MakeRect(X, y, WIDTH, FIELD_HEIGHT),
                                                    "Frame width", frame_width, 1, 4096);
    y += FIELD_STEP;
    const bool HEIGHT_CHANGED = _builder.NumberField(
        ui::MakeId("frame_height"), MakeRect(X, y, WIDTH, FIELD_HEIGHT), "Frame height",
        frame_height, 1, 4096);
    y += FIELD_STEP;
    if (WIDTH_CHANGED || HEIGHT_CHANGED)
    {
        animation.frame_width = static_cast<std::uint32_t>(frame_width);
        animation.frame_height = static_cast<std::uint32_t>(frame_height);
        animation.pivot.x = std::min<std::int32_t>(
            animation.pivot.x, static_cast<std::int32_t>(frame_width));
        animation.pivot.y = std::min<std::int32_t>(
            animation.pivot.y, static_cast<std::int32_t>(frame_height));
        animation.hitbox.width = std::min<std::int32_t>(
            animation.hitbox.width, static_cast<std::int32_t>(frame_width));
        animation.hitbox.height = std::min<std::int32_t>(
            animation.hitbox.height, static_cast<std::int32_t>(frame_height));
        mark_dirty();
    }

    int fps = static_cast<int>(animation.fps);
    if (_builder.NumberField(ui::MakeId("fps"), MakeRect(X, y, WIDTH, FIELD_HEIGHT),
                             "FPS", fps, 1, 240))
    {
        animation.fps = static_cast<std::uint32_t>(fps);
        mark_dirty();
    }
    y += FIELD_STEP;

    int pivot_x = animation.pivot.x;
    int pivot_y = animation.pivot.y;
    const bool PIVOT_X_CHANGED =
        _builder.NumberField(ui::MakeId("pivot_x"), MakeRect(X, y, WIDTH, FIELD_HEIGHT),
                             "Pivot x", pivot_x, 0, 4096);
    y += FIELD_STEP;
    const bool PIVOT_Y_CHANGED =
        _builder.NumberField(ui::MakeId("pivot_y"), MakeRect(X, y, WIDTH, FIELD_HEIGHT),
                             "Pivot y", pivot_y, 0, 4096);
    y += FIELD_STEP;
    if (PIVOT_X_CHANGED || PIVOT_Y_CHANGED)
    {
        animation.pivot.x = pivot_x;
        animation.pivot.y = pivot_y;
        mark_dirty();
    }

    int hitbox_x = animation.hitbox.x;
    int hitbox_y = animation.hitbox.y;
    int hitbox_w = animation.hitbox.width;
    int hitbox_h = animation.hitbox.height;
    if (_builder.NumberField(ui::MakeId("hitbox_x"), MakeRect(X, y, WIDTH, FIELD_HEIGHT),
                             "Hitbox x", hitbox_x, 0, 4096))
    {
        animation.hitbox.x = hitbox_x;
        mark_dirty();
    }
    y += FIELD_STEP;
    if (_builder.NumberField(ui::MakeId("hitbox_y"), MakeRect(X, y, WIDTH, FIELD_HEIGHT),
                             "Hitbox y", hitbox_y, 0, 4096))
    {
        animation.hitbox.y = hitbox_y;
        mark_dirty();
    }
    y += FIELD_STEP;
    if (_builder.NumberField(ui::MakeId("hitbox_w"), MakeRect(X, y, WIDTH, FIELD_HEIGHT),
                             "Hitbox w", hitbox_w, 1, 4096))
    {
        animation.hitbox.width = hitbox_w;
        mark_dirty();
    }
    y += FIELD_STEP;
    if (_builder.NumberField(ui::MakeId("hitbox_h"), MakeRect(X, y, WIDTH, FIELD_HEIGHT),
                             "Hitbox h", hitbox_h, 1, 4096))
    {
        animation.hitbox.height = hitbox_h;
        mark_dirty();
    }
    y += FIELD_STEP;

    if (_builder.Toggle(ui::MakeId("loop"), MakeRect(X, y, WIDTH, 26.0F), "Loop",
                        animation.loop))
    {
        _player.Reset();
        mark_dirty();
    }
    y += 32.0F;
    if (_builder.Toggle(ui::MakeId("playing"), MakeRect(X, y, WIDTH, 26.0F), "Reproducir",
                        _playing))
    {
        _player.Reset();
    }
    y += 32.0F;

    _builder.Text(ui::MakeId("frame_count"), MakeRect(X, y, WIDTH, 18.0F),
                  "Fotogramas: " + std::to_string(animation.frames.size()), TEXT_SCALE,
                  MUTED);
    y += 22.0F;
    if (y + 26.0F < Bottom(rect) - 20.0F &&
        _builder.Button(ui::MakeId("clear_frames"), MakeRect(X, y, WIDTH, 26.0F),
                        "Limpiar fotogramas"))
    {
        animation.frames.clear();
        _player.Reset();
        mark_dirty();
    }
}

void EditorApp::build_level_canvas(const ui::Rect &rect)
{
    _builder.Panel(ui::MakeId("canvas"), rect, CANVAS, GRID);
    LevelDocument *const LEVEL_DOCUMENT = current_level();
    if (LEVEL_DOCUMENT == nullptr)
    {
        _builder.Text(ui::MakeId("canvas_empty"),
                      MakeRect(rect.x + 20.0F, rect.y + 20.0F, rect.width - 40.0F, 40.0F),
                      "Crea o selecciona un nivel.", TEXT_SCALE, MUTED);
        return;
    }

    handle_canvas_input(rect, LEVEL_DOCUMENT->data);
    const Level &level = LEVEL_DOCUMENT->data;
    _builder.SetClip(rect);

    const float CELL_WIDTH =
        static_cast<float>(level.grid.tile_width) * static_cast<float>(_zoom);
    const float CELL_HEIGHT =
        static_cast<float>(level.grid.tile_height) * static_cast<float>(_zoom);
    const float ORIGIN_X = rect.x + static_cast<float>(_pan_x);
    const float ORIGIN_Y = rect.y + static_cast<float>(_pan_y);
    if (CELL_WIDTH <= 0.0F || CELL_HEIGHT <= 0.0F)
    {
        _builder.ResetClip();
        return;
    }

    const int START_COLUMN =
        std::max(0, static_cast<int>(std::floor((rect.x - ORIGIN_X) / CELL_WIDTH)));
    const int START_ROW =
        std::max(0, static_cast<int>(std::floor((rect.y - ORIGIN_Y) / CELL_HEIGHT)));
    const int END_COLUMN =
        std::min(static_cast<int>(level.grid.columns) - 1,
                 static_cast<int>(std::ceil((Right(rect) - ORIGIN_X) / CELL_WIDTH)));
    const int END_ROW =
        std::min(static_cast<int>(level.grid.rows) - 1,
                 static_cast<int>(std::ceil((Bottom(rect) - ORIGIN_Y) / CELL_HEIGHT)));

    for (std::size_t layer_index = 0; layer_index < level.layers.size(); ++layer_index)
    {
        const Layer &LAYER = level.layers[layer_index];
        const bool SELECTED = static_cast<int>(layer_index) == _selected_layer;
        if (!LAYER.visible && (!SELECTED || LAYER.kind != LayerKind::COLLISION))
        {
            continue;
        }
        const Tileset *const TILESET = FindTileset(level, LAYER.tileset_id);
        if (TILESET == nullptr)
        {
            continue;
        }
        const auto *const ENTRY = texture_for(TILESET->image_id);
        if (ENTRY == nullptr)
        {
            continue;
        }
        const ui::Color TINT =
            LAYER.kind == LayerKind::COLLISION ? COLLISION_TINT : ui::Color::White();

        for (int row = START_ROW; row <= END_ROW; ++row)
        {
            for (int column = START_COLUMN; column <= END_COLUMN; ++column)
            {
                const std::size_t INDEX =
                    (static_cast<std::size_t>(row) * level.grid.columns) + column;
                if (INDEX >= LAYER.tiles.size())
                {
                    continue;
                }
                const std::uint32_t TILE = LAYER.tiles[INDEX];
                if (TILE == 0)
                {
                    continue;
                }
                _builder.Image(
                    ui::INVALID_ID,
                    MakeRect(ORIGIN_X + (static_cast<float>(column) * CELL_WIDTH),
                             ORIGIN_Y + (static_cast<float>(row) * CELL_HEIGHT),
                             CELL_WIDTH, CELL_HEIGHT),
                    ENTRY->texture, TileUv(*TILESET, TILE), TINT);
            }
        }
    }

    for (int column = START_COLUMN; column <= END_COLUMN + 1; ++column)
    {
        const float LINE_X = ORIGIN_X + (static_cast<float>(column) * CELL_WIDTH);
        const float TOP = std::max(rect.y, ORIGIN_Y);
        const float BOTTOM =
            std::min(Bottom(rect), ORIGIN_Y + (level.grid.rows * CELL_HEIGHT));
        if (BOTTOM > TOP)
        {
            _builder.FillRect(ui::INVALID_ID, MakeRect(LINE_X, TOP, 1.0F, BOTTOM - TOP),
                              GRID);
        }
    }
    for (int row = START_ROW; row <= END_ROW + 1; ++row)
    {
        const float LINE_Y = ORIGIN_Y + (static_cast<float>(row) * CELL_HEIGHT);
        const float LEFT = std::max(rect.x, ORIGIN_X);
        const float RIGHT =
            std::min(Right(rect), ORIGIN_X + (level.grid.columns * CELL_WIDTH));
        if (RIGHT > LEFT)
        {
            _builder.FillRect(ui::INVALID_ID, MakeRect(LEFT, LINE_Y, RIGHT - LEFT, 1.0F),
                              GRID);
        }
    }

    if (rect.Contains(_input.mouse))
    {
        const int COLUMN =
            static_cast<int>(std::floor((_input.mouse.x - ORIGIN_X) / CELL_WIDTH));
        const int ROW =
            static_cast<int>(std::floor((_input.mouse.y - ORIGIN_Y) / CELL_HEIGHT));
        if (COLUMN >= 0 && COLUMN < static_cast<int>(level.grid.columns) && ROW >= 0 &&
            ROW < static_cast<int>(level.grid.rows))
        {
            const ui::Rect CELL{ORIGIN_X + (static_cast<float>(COLUMN) * CELL_WIDTH),
                                ORIGIN_Y + (static_cast<float>(ROW) * CELL_HEIGHT),
                                CELL_WIDTH, CELL_HEIGHT};
            constexpr float BORDER = 2.0F;
            _builder.FillRect(ui::INVALID_ID,
                              MakeRect(CELL.x, CELL.y, CELL.width, BORDER),
                              SELECTION_COLOR);
            _builder.FillRect(
                ui::INVALID_ID,
                MakeRect(CELL.x, CELL.y + CELL.height - BORDER, CELL.width, BORDER),
                SELECTION_COLOR);
            _builder.FillRect(ui::INVALID_ID,
                              MakeRect(CELL.x, CELL.y, BORDER, CELL.height),
                              SELECTION_COLOR);
            _builder.FillRect(
                ui::INVALID_ID,
                MakeRect(CELL.x + CELL.width - BORDER, CELL.y, BORDER, CELL.height),
                SELECTION_COLOR);
        }
    }

    _builder.ResetClip();
    _builder.Text(
        ui::MakeId("canvas_info"),
        MakeRect(rect.x + 8.0F, Bottom(rect) - 22.0F, rect.width - 16.0F, 18.0F),
        "Zoom " + std::to_string(static_cast<int>(_zoom * 100.0F)) +
            "%  |  Rueda: zoom, boton central: pan, clic: pintar, "
            "clic derecho: borrar",
        TEXT_SCALE, MUTED);
}

void EditorApp::build_animation_canvas(const ui::Rect &rect)
{
    _builder.Panel(ui::MakeId("animation_canvas"), rect, CANVAS, GRID);
    AnimationDocument *const ANIMATION_DOCUMENT = current_animation();
    if (ANIMATION_DOCUMENT == nullptr)
    {
        _builder.Text(ui::MakeId("animation_canvas_empty"),
                      MakeRect(rect.x + 20.0F, rect.y + 20.0F, rect.width - 40.0F, 40.0F),
                      "Crea o selecciona una animación.", TEXT_SCALE, MUTED);
        return;
    }

    const Animation &animation = ANIMATION_DOCUMENT->data;
    if (animation.frames.empty())
    {
        _builder.Text(ui::MakeId("animation_no_frames"),
                      MakeRect(rect.x + 20.0F, rect.y + 20.0F, rect.width - 40.0F, 40.0F),
                      "Selecciona fotogramas en la tira inferior.", TEXT_SCALE, MUTED);
        return;
    }

    const auto *const ENTRY = texture_for(animation.image_id);
    if (ENTRY == nullptr)
    {
        return;
    }
    const Frame *const FRAME = _player.CurrentFrame(animation);
    if (FRAME == nullptr)
    {
        return;
    }

    _builder.SetClip(rect);
    float scale = 1.0F;
    const ui::Rect DRAW_RECT = FitRect(rect, FRAME->width, FRAME->height, 40.0F, scale);
    (void)scale;
    _builder.Image(ui::MakeId("animation_preview"), DRAW_RECT, ENTRY->texture,
                   PixelUv(*FRAME, ENTRY->width, ENTRY->height), ui::Color::White());

    const float SCALE_X =
        DRAW_RECT.width / static_cast<float>(std::max(animation.frame_width, 1U));
    const float SCALE_Y =
        DRAW_RECT.height / static_cast<float>(std::max(animation.frame_height, 1U));

    const float PIVOT_X = DRAW_RECT.x + (animation.pivot.x * SCALE_X);
    const float PIVOT_Y = DRAW_RECT.y + (animation.pivot.y * SCALE_Y);
    _builder.FillRect(ui::INVALID_ID, MakeRect(PIVOT_X - 6.0F, PIVOT_Y, 12.0F, 1.0F),
                      SELECTION_COLOR);
    _builder.FillRect(ui::INVALID_ID, MakeRect(PIVOT_X, PIVOT_Y - 6.0F, 1.0F, 12.0F),
                      SELECTION_COLOR);

    const ui::Rect HITBOX{DRAW_RECT.x + (animation.hitbox.x * SCALE_X),
                          DRAW_RECT.y + (animation.hitbox.y * SCALE_Y),
                          animation.hitbox.width * SCALE_X,
                          animation.hitbox.height * SCALE_Y};
    _builder.FillRect(ui::INVALID_ID, MakeRect(HITBOX.x, HITBOX.y, HITBOX.width, 1.0F),
                      ERROR_COLOR);
    _builder.FillRect(ui::INVALID_ID,
                      MakeRect(HITBOX.x, HITBOX.y + HITBOX.height, HITBOX.width, 1.0F),
                      ERROR_COLOR);
    _builder.FillRect(ui::INVALID_ID, MakeRect(HITBOX.x, HITBOX.y, 1.0F, HITBOX.height),
                      ERROR_COLOR);
    _builder.FillRect(ui::INVALID_ID,
                      MakeRect(HITBOX.x + HITBOX.width, HITBOX.y, 1.0F, HITBOX.height),
                      ERROR_COLOR);

    _builder.ResetClip();
    _builder.Text(
        ui::MakeId("animation_info"),
        MakeRect(rect.x + 8.0F, Bottom(rect) - 22.0F, rect.width - 16.0F, 18.0F),
        "Frame " + std::to_string(_player.CurrentFrameIndex() + 1) + "/" +
            std::to_string(animation.frames.size()) + "  |  " +
            std::to_string(animation.fps) + " fps",
        TEXT_SCALE, MUTED);
}

void EditorApp::build_level_palette(const ui::Rect &rect)
{
    _builder.Panel(ui::MakeId("palette"), rect, PANEL, GRID);
    LevelDocument *const LEVEL_DOCUMENT = current_level();
    if (LEVEL_DOCUMENT == nullptr || LEVEL_DOCUMENT->data.layers.empty())
    {
        return;
    }
    const Level &level = LEVEL_DOCUMENT->data;
    if (_selected_layer < 0 ||
        static_cast<std::size_t>(_selected_layer) >= level.layers.size())
    {
        return;
    }
    const Layer &layer = level.layers[static_cast<std::size_t>(_selected_layer)];
    const Tileset *const TILESET = FindTileset(level, layer.tileset_id);
    if (TILESET == nullptr)
    {
        return;
    }
    const auto *const ENTRY = texture_for(TILESET->image_id);
    if (ENTRY == nullptr)
    {
        return;
    }

    float scale = 1.0F;
    const ui::Rect IMAGE_RECT =
        FitRect(rect, ENTRY->width, ENTRY->height, PADDING, scale);
    handle_palette_input(IMAGE_RECT, level);

    _builder.SetClip(rect);
    _builder.Image(ui::MakeId("palette_image"), IMAGE_RECT, ENTRY->texture, ui::UvRect{},
                   ui::Color::White());

    if (TILESET->columns <= 64 && TILESET->rows <= 64)
    {
        const float TILE_WIDTH =
            (IMAGE_RECT.width / static_cast<float>(TILESET->columns));
        const float TILE_HEIGHT = (IMAGE_RECT.height / static_cast<float>(TILESET->rows));
        for (std::uint32_t column = 0; column <= TILESET->columns; ++column)
        {
            _builder.FillRect(ui::INVALID_ID,
                              MakeRect(IMAGE_RECT.x + (column * TILE_WIDTH), IMAGE_RECT.y,
                                       1.0F, IMAGE_RECT.height),
                              GRID);
        }
        for (std::uint32_t row = 0; row <= TILESET->rows; ++row)
        {
            _builder.FillRect(ui::INVALID_ID,
                              MakeRect(IMAGE_RECT.x, IMAGE_RECT.y + (row * TILE_HEIGHT),
                                       IMAGE_RECT.width, 1.0F),
                              GRID);
        }
        if (_selected_tile > 0 && _selected_tile <= TILESET->columns * TILESET->rows)
        {
            const std::uint32_t ZERO_BASED = _selected_tile - 1U;
            const std::uint32_t COLUMN = ZERO_BASED % TILESET->columns;
            const std::uint32_t ROW = ZERO_BASED / TILESET->columns;
            _builder.FillRect(ui::INVALID_ID,
                              MakeRect(IMAGE_RECT.x + (COLUMN * TILE_WIDTH),
                                       IMAGE_RECT.y + (ROW * TILE_HEIGHT), TILE_WIDTH,
                                       2.0F),
                              SELECTION_COLOR);
            _builder.FillRect(ui::INVALID_ID,
                              MakeRect(IMAGE_RECT.x + (COLUMN * TILE_WIDTH),
                                       IMAGE_RECT.y + ((ROW + 1U) * TILE_HEIGHT) - 2.0F,
                                       TILE_WIDTH, 2.0F),
                              SELECTION_COLOR);
        }
    }

    _builder.ResetClip();
    _builder.Text(ui::MakeId("palette_info"),
                  MakeRect(rect.x + 8.0F, rect.y + 4.0F, rect.width - 16.0F, 18.0F),
                  "Tile " + std::to_string(_selected_tile) +
                      (_eraser ? "  (borrador)" : ""),
                  TEXT_SCALE, MUTED);
}

void EditorApp::build_animation_sheet(const ui::Rect &rect)
{
    _builder.Panel(ui::MakeId("sheet"), rect, PANEL, GRID);
    AnimationDocument *const ANIMATION_DOCUMENT = current_animation();
    if (ANIMATION_DOCUMENT == nullptr)
    {
        return;
    }
    Animation &animation = ANIMATION_DOCUMENT->data;
    const auto *const ENTRY = texture_for(animation.image_id);
    if (ENTRY == nullptr || animation.frame_width == 0 || animation.frame_height == 0)
    {
        return;
    }

    float scale = 1.0F;
    const ui::Rect IMAGE_RECT =
        FitRect(rect, ENTRY->width, ENTRY->height, PADDING, scale);
    handle_sheet_input(IMAGE_RECT, animation, ENTRY->width, ENTRY->height);

    _builder.SetClip(rect);
    _builder.Image(ui::MakeId("sheet_image"), IMAGE_RECT, ENTRY->texture, ui::UvRect{},
                   ui::Color::White());

    const std::uint32_t COLUMNS = ENTRY->width / animation.frame_width;
    const std::uint32_t ROWS = ENTRY->height / animation.frame_height;
    if (COLUMNS > 0 && ROWS > 0 && COLUMNS <= 64 && ROWS <= 64)
    {
        const float CELL_WIDTH = IMAGE_RECT.width / static_cast<float>(COLUMNS);
        const float CELL_HEIGHT = IMAGE_RECT.height / static_cast<float>(ROWS);
        for (std::uint32_t column = 0; column <= COLUMNS; ++column)
        {
            _builder.FillRect(ui::INVALID_ID,
                              MakeRect(IMAGE_RECT.x + (column * CELL_WIDTH), IMAGE_RECT.y,
                                       1.0F, IMAGE_RECT.height),
                              GRID);
        }
        for (std::uint32_t row = 0; row <= ROWS; ++row)
        {
            _builder.FillRect(ui::INVALID_ID,
                              MakeRect(IMAGE_RECT.x, IMAGE_RECT.y + (row * CELL_HEIGHT),
                                       IMAGE_RECT.width, 1.0F),
                              GRID);
        }

        for (std::uint32_t row = 0; row < ROWS; ++row)
        {
            for (std::uint32_t column = 0; column < COLUMNS; ++column)
            {
                const Frame CELL{column * animation.frame_width,
                                 row * animation.frame_height, animation.frame_width,
                                 animation.frame_height, 0};
                const auto ITERATOR = std::find_if(
                    animation.frames.begin(), animation.frames.end(),
                    [&CELL](const Frame &frame) { return SameFrame(frame, CELL); });
                if (ITERATOR != animation.frames.end())
                {
                    _builder.FillRect(ui::INVALID_ID,
                                      MakeRect(IMAGE_RECT.x + (column * CELL_WIDTH),
                                               IMAGE_RECT.y + (row * CELL_HEIGHT),
                                               CELL_WIDTH, 2.0F),
                                      SELECTION_COLOR);
                    _builder.FillRect(
                        ui::INVALID_ID,
                        MakeRect(IMAGE_RECT.x + (column * CELL_WIDTH),
                                 IMAGE_RECT.y + ((row + 1U) * CELL_HEIGHT) - 2.0F,
                                 CELL_WIDTH, 2.0F),
                        SELECTION_COLOR);
                }
            }
        }
    }

    _builder.ResetClip();
    _builder.Text(ui::MakeId("sheet_info"),
                  MakeRect(rect.x + 8.0F, rect.y + 4.0F, rect.width - 16.0F, 18.0F),
                  "Fotogramas: " + std::to_string(animation.frames.size()) +
                      "  |  clic para anadir o quitar",
                  TEXT_SCALE, MUTED);
}

void EditorApp::handle_canvas_input(const ui::Rect &rect, const Level &level)
{
    if (!rect.Contains(_input.mouse))
    {
        if (_input.left_released || _input.right_released)
        {
            _canvas_painting = false;
        }
        return;
    }

    const float CELL_WIDTH =
        static_cast<float>(level.grid.tile_width) * static_cast<float>(_zoom);
    const float CELL_HEIGHT =
        static_cast<float>(level.grid.tile_height) * static_cast<float>(_zoom);
    if (CELL_WIDTH <= 0.0F || CELL_HEIGHT <= 0.0F)
    {
        return;
    }

    if (_input.wheel != 0.0F)
    {
        const float OLD_ZOOM = static_cast<float>(_zoom);
        float new_zoom = OLD_ZOOM * std::pow(1.1F, _input.wheel);
        new_zoom = std::clamp(new_zoom, MIN_ZOOM, MAX_ZOOM);
        const float LOCAL_X = _input.mouse.x - rect.x;
        const float LOCAL_Y = _input.mouse.y - rect.y;
        const float WORLD_X = (LOCAL_X - static_cast<float>(_pan_x)) / OLD_ZOOM;
        const float WORLD_Y = (LOCAL_Y - static_cast<float>(_pan_y)) / OLD_ZOOM;
        _zoom = new_zoom;
        _pan_x = LOCAL_X - (WORLD_X * new_zoom);
        _pan_y = LOCAL_Y - (WORLD_Y * new_zoom);
    }

    if (_input.middle_down)
    {
        _pan_x += _input.mouse.x - _previous_mouse.x;
        _pan_y += _input.mouse.y - _previous_mouse.y;
    }

    if (_input.left_pressed)
    {
        _canvas_painting = true;
        _canvas_erasing = _eraser;
        _last_painted_valid = false;
    }
    if (_input.right_pressed)
    {
        _canvas_painting = true;
        _canvas_erasing = true;
        _last_painted_valid = false;
    }
    if (_input.left_released || _input.right_released)
    {
        _canvas_painting = false;
    }

    if (!_canvas_painting)
    {
        return;
    }

    const float ORIGIN_X = rect.x + static_cast<float>(_pan_x);
    const float ORIGIN_Y = rect.y + static_cast<float>(_pan_y);
    const int COLUMN =
        static_cast<int>(std::floor((_input.mouse.x - ORIGIN_X) / CELL_WIDTH));
    const int ROW =
        static_cast<int>(std::floor((_input.mouse.y - ORIGIN_Y) / CELL_HEIGHT));
    if (COLUMN < 0 || COLUMN >= static_cast<int>(level.grid.columns) || ROW < 0 ||
        ROW >= static_cast<int>(level.grid.rows))
    {
        _last_painted_valid = false;
        return;
    }

    const auto COLUMN_VALUE = static_cast<std::uint32_t>(COLUMN);
    const auto ROW_VALUE = static_cast<std::uint32_t>(ROW);
    if (_last_painted_valid)
    {
        paint_canvas_line(_last_painted_column, _last_painted_row, COLUMN_VALUE,
                          ROW_VALUE, _canvas_erasing);
    }
    else
    {
        paint_canvas_cell(COLUMN_VALUE, ROW_VALUE, _canvas_erasing);
    }
    _last_painted_valid = true;
    _last_painted_column = COLUMN_VALUE;
    _last_painted_row = ROW_VALUE;
}

void EditorApp::paint_canvas_cell(std::uint32_t column, std::uint32_t row, bool erase)
{
    LevelDocument *const LEVEL_DOCUMENT = current_level();
    if (LEVEL_DOCUMENT == nullptr || _selected_layer < 0 ||
        static_cast<std::size_t>(_selected_layer) >= LEVEL_DOCUMENT->data.layers.size())
    {
        return;
    }
    Level &level = LEVEL_DOCUMENT->data;
    Layer &layer = level.layers[static_cast<std::size_t>(_selected_layer)];
    const std::size_t INDEX =
        (static_cast<std::size_t>(row) * level.grid.columns) + column;
    if (INDEX >= layer.tiles.size())
    {
        return;
    }
    const std::uint32_t NEW_TILE = erase ? 0U : _selected_tile;
    if (layer.tiles[INDEX] == NEW_TILE)
    {
        return;
    }
    if (PaintTile(level, static_cast<std::size_t>(_selected_layer), column, row,
                  NEW_TILE))
    {
        std::cout << "Tile " << NEW_TILE << " en capa " << _selected_layer << " ("
                  << column << ", " << row << ").\n";
        mark_dirty();
    }
}

void EditorApp::paint_canvas_line(std::uint32_t from_column, std::uint32_t from_row,
                                  std::uint32_t to_column, std::uint32_t to_row,
                                  bool erase)
{
    int x0 = static_cast<int>(from_column);
    int y0 = static_cast<int>(from_row);
    const int X1 = static_cast<int>(to_column);
    const int Y1 = static_cast<int>(to_row);
    const int DX = std::abs(X1 - x0);
    const int DY = -std::abs(Y1 - y0);
    const int STEP_X = x0 < X1 ? 1 : -1;
    const int STEP_Y = y0 < Y1 ? 1 : -1;
    int error = DX + DY;

    while (true)
    {
        paint_canvas_cell(static_cast<std::uint32_t>(x0), static_cast<std::uint32_t>(y0),
                          erase);
        if (x0 == X1 && y0 == Y1)
        {
            break;
        }
        const int DOUBLE_ERROR = 2 * error;
        if (DOUBLE_ERROR >= DY)
        {
            error += DY;
            x0 += STEP_X;
        }
        if (DOUBLE_ERROR <= DX)
        {
            error += DX;
            y0 += STEP_Y;
        }
    }
}

void EditorApp::handle_palette_input(const ui::Rect &rect, const Level &level)
{
    if (!_input.left_pressed || !rect.Contains(_input.mouse))
    {
        return;
    }
    if (_selected_layer < 0 ||
        static_cast<std::size_t>(_selected_layer) >= level.layers.size())
    {
        return;
    }
    const Layer &layer = level.layers[static_cast<std::size_t>(_selected_layer)];
    const Tileset *const TILESET = FindTileset(level, layer.tileset_id);
    if (TILESET == nullptr || TILESET->columns == 0 || TILESET->rows == 0)
    {
        return;
    }
    const float TILE_WIDTH = rect.width / static_cast<float>(TILESET->columns);
    const float TILE_HEIGHT = rect.height / static_cast<float>(TILESET->rows);
    const int COLUMN =
        static_cast<int>(std::floor((_input.mouse.x - rect.x) / TILE_WIDTH));
    const int ROW = static_cast<int>(std::floor((_input.mouse.y - rect.y) / TILE_HEIGHT));
    if (COLUMN < 0 || COLUMN >= static_cast<int>(TILESET->columns) || ROW < 0 ||
        ROW >= static_cast<int>(TILESET->rows))
    {
        return;
    }
    std::uint32_t tile = 0;
    if (PickTile(*TILESET, static_cast<std::uint32_t>(COLUMN),
                 static_cast<std::uint32_t>(ROW), tile))
    {
        _selected_tile = tile;
        _eraser = false;
    }
}

void EditorApp::handle_sheet_input(const ui::Rect &rect, Animation &animation,
                                   std::uint32_t image_width, std::uint32_t image_height)
{
    if (!_input.left_pressed || !rect.Contains(_input.mouse) ||
        animation.frame_width == 0 || animation.frame_height == 0)
    {
        return;
    }
    const std::uint32_t COLUMNS = image_width / animation.frame_width;
    const std::uint32_t ROWS = image_height / animation.frame_height;
    if (COLUMNS == 0 || ROWS == 0)
    {
        return;
    }
    const float CELL_WIDTH = rect.width / static_cast<float>(COLUMNS);
    const float CELL_HEIGHT = rect.height / static_cast<float>(ROWS);
    const int COLUMN =
        static_cast<int>(std::floor((_input.mouse.x - rect.x) / CELL_WIDTH));
    const int ROW = static_cast<int>(std::floor((_input.mouse.y - rect.y) / CELL_HEIGHT));
    if (COLUMN < 0 || COLUMN >= static_cast<int>(COLUMNS) || ROW < 0 ||
        ROW >= static_cast<int>(ROWS))
    {
        return;
    }

    const Frame CELL{static_cast<std::uint32_t>(COLUMN) * animation.frame_width,
                     static_cast<std::uint32_t>(ROW) * animation.frame_height,
                     animation.frame_width, animation.frame_height, 0};
    const auto ITERATOR =
        std::find_if(animation.frames.begin(), animation.frames.end(),
                     [&CELL](const Frame &frame) { return SameFrame(frame, CELL); });
    if (ITERATOR == animation.frames.end())
    {
        if (animation.frames.size() < MAX_FRAMES)
        {
            animation.frames.push_back(CELL);
        }
    }
    else
    {
        animation.frames.erase(ITERATOR);
    }
    _player.Reset();
    mark_dirty();
}

} // namespace k_engine::editor

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-identifier-naming,readability-identifier-length,bugprone-easily-swappable-parameters,bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions,modernize-use-auto,cppcoreguidelines-pro-bounds-pointer-arithmetic,bugprone-branch-clone,readability-function-cognitive-complexity)
