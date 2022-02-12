#ifndef PIXA_SCENE_H
#define PIXA_SCENE_H

typedef struct
{
    bool is_active;

    void (*on_create)();
    void (*on_update)();
    void (*on_destroy)();
} Scene;

/*! @brief Create a scene.
 *  @param[in] on_create The callback that is called when the scene is created, or `NULL` if no callback is needed
 *  @param[in] on_update The callback that is called each frame
 *  @param[in] on_destroy The callback that is called when the scene is destroyed, or `NULL` if no callback is needed
 *  @return The ID of the scene
 */
int scene_create(void (*on_create)(), void (*on_update)(), void (*on_destroy)());
/*! @brief Destory a scene.
 *  @param[in] id The ID of the new scene.
 */
void scene_destroy(int id);

#endif