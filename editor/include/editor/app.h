#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_set>

#include "editor/animation_player.h"
#include "editor/cli.h"
#include "editor/document.h"
#include "editor/gfx/quad_renderer.h"
#include "editor/gfx/texture_cache.h"
#include "editor/gfx/vulkan_context.h"
#include "ui/builder.h"

struct GLFWwindow;

namespace k_engine::editor
{

class EditorApp
{
  public:
    EditorApp();
    ~EditorApp();

    EditorApp(const EditorApp &) = delete;
    EditorApp &operator=(const EditorApp &) = delete;
    EditorApp(EditorApp &&) = delete;
    EditorApp &operator=(EditorApp &&) = delete;

    bool Initialize(const EditorOptions &options);
    int Run(const EditorOptions &options);
    void Shutdown() noexcept;

  private:
    enum class Mode : std::uint8_t
    {
        LEVEL = 0,
        ANIMATION = 1,
    };

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                             int mods);
    static void char_callback(GLFWwindow *window, unsigned int codepoint);
    static void cursor_position_callback(GLFWwindow *window, double cursor_x,
                                         double cursor_y);
    static void mouse_button_callback(GLFWwindow *window, int button, int action,
                                      int mods);
    static void scroll_callback(GLFWwindow *window, double offset_x, double offset_y);
    static void drop_callback(GLFWwindow *window, int count, const char **paths);

    void install_callbacks();
    void reset_transient_input() noexcept;

    void build_ui();
    void build_top_bar(const ui::Rect &rect);
    void build_left_panel(const ui::Rect &rect);
    void build_right_panel(const ui::Rect &rect);
    void build_level_properties(const ui::Rect &rect);
    void build_animation_properties(const ui::Rect &rect);
    void build_level_canvas(const ui::Rect &rect);
    void build_animation_canvas(const ui::Rect &rect);
    void build_level_palette(const ui::Rect &rect);
    void build_animation_sheet(const ui::Rect &rect);

    void paint_canvas_cell(std::uint32_t column, std::uint32_t row, bool erase);
    void paint_canvas_line(std::uint32_t from_column, std::uint32_t from_row,
                           std::uint32_t to_column, std::uint32_t to_row, bool erase);
    void handle_canvas_input(const ui::Rect &rect, const Level &level);
    void handle_palette_input(const ui::Rect &rect, const Level &level);
    void handle_sheet_input(const ui::Rect &rect, Animation &animation,
                            std::uint32_t image_width, std::uint32_t image_height);

    void save_project();
    void import_png(const std::filesystem::path &path);
    void create_level();
    void create_animation();
    void add_layer(LayerKind kind);
    void delete_selected_layer();
    void add_tileset();
    void set_grid_tile_size(std::uint32_t tile_width, std::uint32_t tile_height);
    void set_grid_dimensions(std::uint32_t columns, std::uint32_t rows);

    LevelDocument *current_level() noexcept;
    [[nodiscard]] const LevelDocument *current_level() const noexcept;
    AnimationDocument *current_animation() noexcept;
    [[nodiscard]] const AnimationDocument *current_animation() const noexcept;
    [[nodiscard]] const gfx::TextureCache::Entry *texture_for(std::uint64_t asset_id);
    void refresh_selection() noexcept;
    void set_status(std::string message, bool error);
    void mark_dirty() noexcept;
    void try_save_before_close();
    void ensure_textures();

    gfx::VulkanContext _context;
    gfx::QuadRenderer _renderer;
    gfx::TextureCache _textures;
    ui::Builder _builder;

    EditorOptions _options;
    ProjectBundle _bundle;
    bool _has_project = false;

    Mode _mode = Mode::LEVEL;
    int _selected_level = -1;
    int _selected_animation = -1;
    int _selected_layer = 0;
    int _selected_asset = -1;
    int _selected_tileset = 0;
    std::uint32_t _selected_tile = 1;
    bool _eraser = false;
    bool _playing = true;

    double _zoom = 1.0;
    double _pan_x = 0.0;
    double _pan_y = 0.0;
    ui::Vec2 _previous_mouse{};
    bool _last_painted_valid = false;
    std::uint32_t _last_painted_column = 0;
    std::uint32_t _last_painted_row = 0;
    bool _canvas_painting = false;
    bool _canvas_erasing = false;

    AnimationPlayer _player;
    double _last_frame_time = 0.0;

    std::string _status;
    bool _status_error = false;
    bool _save_requested = false;
    bool _shutdown_complete = true;

    std::uint64_t _render_generation = 0;
    std::unordered_set<std::uint64_t> _failed_textures;

    ui::Input _input;
    bool _ctrl_pressed = false;
    bool _shift_pressed = false;
};

} // namespace k_engine::editor
