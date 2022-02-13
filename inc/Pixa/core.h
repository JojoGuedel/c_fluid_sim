#ifndef PIXA_CORE_H
#define PIXA_CORE_H

#include <stdbool.h>

extern bool active;

extern int width;
extern int height;

extern double mouse_x;
extern double mouse_y;

extern double elapsed_time;
extern double delta_time;

/*! @brief Create the engine.
 *  @param[in] width The width of the window
 *  @param[in] height The height of the window
 *  @param[in] res_x The amount of screen-pixels per base-layer-pixel in x direction
 *  @param[in] res_y The amount of screen-pixels per base-layer-pixel in y direction
 */
void engine_create(int width, int height, int res_x, int res_y);
/*! @brief Destory the engine.
 */
void engine_destroy();

/*! @brief Start the engine. */
void engine_start();
/*! @brief Stop the engine. */
void engine_stop();

void engine_set_user_input(void (*on_key_pressed)(int key, int action, int flags), void (*on_mouse_pressed)(int button, int action, int flags));

/*! @brief Get the width of the currently bound layer.
 *  @return The width of currently bound layer.
 */
int get_width();
/*! @brief Get the height of the currently bound layer.
 *  @return The height of the currently bound layer.
*/

#endif