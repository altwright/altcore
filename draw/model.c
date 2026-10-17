//
// Created by wright on 10/17/26.
//

#include "model.h"

#include <string.h>

#include "../debug.h"
#include "../memory.h"
#include "model/glb.h"

typedef struct INDICES_T {
    IndiceType type;

    union {
        u16s shorts;
        u32s longs;
    };
} Indices;

Model model_load(ModelLoadInfo *info) {
    Model model = {};

    Arena *tmp = arena_make(MIBIBYTE);

    switch (info->type) {
        case MODEL_DATA_TYPE_GLB: {
            cgltf_options glb_options = {
                .memory = {
                    .alloc_func = glb_alloc,
                    .free_func = glb_free,
                }
            };
            cgltf_data *glb_data = nullptr;
            cgltf_result glb_result = cgltf_parse(&glb_options, info->data.bytes, info->data.len, &glb_data);
            if (glb_result != cgltf_result_success) {
                crash_msg("Failed to parse GLB data\n");
            }

            if (cgltf_validate(glb_data) != cgltf_result_success) {
                crash_msg("GLB data is invalid\n");
            }

            if (cgltf_load_buffers(&glb_options, glb_data, nullptr) != cgltf_result_success) {
                crash_msg("Failed to load GLB buffers\n");
            }

            if (glb_data->scenes_count > 1) {
                crash_msg("Greater than one scene in GLB\n");
            }

            cgltf_scene *scene = glb_data->scene;

            if (glb_data->nodes_count != scene->nodes_count) {
                crash_msg("Node counts do not match\n");
            }

            model.arena = arena_make(MIBIBYTE);
            model.meshes = (Meshes){
                .arena = model.arena,
                .len = (i64) scene->nodes_count,
            };
            ARRAY_MAKE(&model.meshes);
            ARRAY_FOR(mesh, &model.meshes) {
                mesh->parent_idx = -1;
            }

            for (i64 node_idx = 0; node_idx < scene->nodes_count; node_idx++) {
                Mesh *mesh = ARRAY_GET(&model.meshes, node_idx);
                cgltf_node *node = scene->nodes[node_idx];

                if (!node->mesh) {
                    crash_msg("Node does not have a mesh\n");
                }

                if (node->parent) {
                    mesh->parent_idx = node->parent - glb_data->nodes;
                }

                memcpy(mesh->local.translation.data, node->translation, sizeof(node->translation));
                memcpy(mesh->local.rotation.data, node->rotation, sizeof(node->rotation));
                memcpy(mesh->local.scale.data, node->scale, sizeof(node->scale));

                if (node->name) {
                    mesh->name = string_make(model.arena, "%s", node->name);
                }

                mesh->prims = (MeshPrimitives){
                    .arena = model.arena,
                    .len = (i64) node->mesh->primitives_count,
                };
                ARRAY_MAKE(&mesh->prims);

                for (i64 prim_idx = 0; prim_idx < node->mesh->primitives_count; prim_idx++) {
                    MeshPrimitive *prim = ARRAY_GET(&mesh->prims, prim_idx);
                    cgltf_primitive node_mesh_prim = node->mesh->primitives[prim_idx];

                    switch (node_mesh_prim.type) {
                        case cgltf_primitive_type_triangles: {
                            prim->type = MESH_PRIMITIVE_TYPE_TRIANGLES;
                            break;
                        }
                        default:
                            crash_msg("Unhandled GLB primitive type\n");
                            break;
                    }

                    Indices indices = {};
                    switch (node_mesh_prim.indices->component_type) {
                        case cgltf_component_type_r_16u: {
                            indices.type = INDICE_TYPE_U16;
                            indices.shorts = (u16s){
                                .arena = tmp,
                                .len = (i64) node_mesh_prim.indices->count,
                            };
                            ARRAY_MAKE(&indices.shorts);

                            cgltf_accessor_unpack_indices(
                                node_mesh_prim.indices,
                                indices.shorts.data,
                                sizeof(u16),
                                indices.shorts.len);

                            break;
                        }
                        case cgltf_component_type_r_32u: {
                            indices.type = INDICE_TYPE_U32;
                            indices.longs = (u32s){
                                .arena = tmp,
                                .len = (i64) node_mesh_prim.indices->count,
                            };
                            ARRAY_MAKE(&indices.longs);

                            cgltf_accessor_unpack_indices(
                                node_mesh_prim.indices,
                                indices.longs.data,
                                sizeof(u32),
                                indices.longs.len);

                            break;
                        }
                        default:
                            crash_msg("Unhandled indices component type %d\n", node_mesh_prim.indices->component_type);
                            break;
                    }

                    const cgltf_accessor *pos_accessor = cgltf_find_accessor(
                        &node_mesh_prim, cgltf_attribute_type_position, 0);
                    if (!pos_accessor) {
                        crash_msg("No position data for mesh primitive vertices found\n");
                    }

                    if (pos_accessor->type != cgltf_type_vec3) {
                        crash_msg("Expected position data to be vec3, not %d\n", pos_accessor->type);
                    }

                    if (pos_accessor->component_type != cgltf_component_type_r_32f) {
                        crash_msg("Expected position data components to be f32, not %d\n",
                                  pos_accessor->component_type);
                    }

                    f32s vert_components = {
                        .arena = tmp,
                        .len = (i64) pos_accessor->count * 3,
                    };
                    ARRAY_MAKE(&vert_components);

                    cgltf_accessor_unpack_floats(pos_accessor, vert_components.data, vert_components.len);

                    IndexBufferCreateInfo index_buffer_info = {
                        .renderer = info->renderer,
                        .indices = {
                            .type = indices.type,
                        }
                    };

                    switch (indices.type) {
                        case INDICE_TYPE_U16: {
                            index_buffer_info.indices.count = indices.shorts.len;
                            index_buffer_info.indices.start = (u8 *) indices.shorts.data;
                            index_buffer_info.indices.stride_bytes = sizeof(u16);
                            break;
                        }
                        case INDICE_TYPE_U32: {
                            index_buffer_info.indices.count = indices.longs.len;
                            index_buffer_info.indices.start = (u8 *) indices.longs.data;
                            index_buffer_info.indices.stride_bytes = sizeof(u32);
                            break;
                        }
                        default:
                            crash_msg("Unhandled indice type\n", indices.type);
                            break;
                    }

                    prim->indices = index_buffer_create(&index_buffer_info);

                    VertexBufferCreateInfo vertex_buffer_info = {
                        .renderer = info->renderer,
                        .vertices = {
                            .type = VERTEX_COMPONENT_TYPE_F32,
                            .start = (u8 *) vert_components.data,
                            .count = vert_components.len / 3,
                            .stride_bytes = sizeof(f32) * 3,
                        }
                    };

                    prim->vertices = vertex_buffer_create(&vertex_buffer_info);
                }
            }

            cgltf_free(glb_data);
            break;
        }
        default:
            crash_msg("Unhandled model data type %d\n", info->type);
            break;
    }

    arena_free(tmp);

    return model;
}

void model_unload(Model *model) {
    ARRAY_FOR(mesh, &model->meshes) {
        ARRAY_FOR(prim, &mesh->prims) {
            vertex_buffer_destroy(prim->vertices);
            index_buffer_destroy(prim->indices);
        }
    }
    arena_free(model->arena);
}
