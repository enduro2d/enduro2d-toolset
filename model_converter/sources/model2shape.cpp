/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019 Matvey Cherevko
 ******************************************************************************/

#include <cassert>
#include <cstring>
#include <cstdint>

#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <exception>
#include <stdexcept>
#include <algorithm>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace
{
    const std::uint32_t shape_file_version = 1;
    const std::string shape_file_signature = "e2d_shape";

    struct opts {
        bool timers = false;
        bool verbose = false;

        opts(int argc, char *argv[]) {
            timers = has_flag("-t", argc, argv) || has_flag("--timers", argc, argv);
            verbose = has_flag("-v", argc, argv) || has_flag("--verbose", argc, argv);
        }

    private:
        static bool has_flag(const char* flag, int argc, char *argv[]) noexcept {
            for ( int i = 0; i < argc; ++i ) {
                if ( 0 == std::strcmp(argv[i], flag) ) {
                    return true;
                }
            }
            return false;
        }
    };

    class timer {
    public:
        timer()
        : tp_(std::chrono::high_resolution_clock::now()) {}

        void done() const {
            const auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - tp_);
            std::cout << duration_us.count() << "us" << std::endl;
        }
    private:
        std::chrono::high_resolution_clock::time_point tp_;
    };

    struct v2f {
        float x = 0.f;
        float y = 0.f;

        v2f(float nx, float ny)
        : x(nx), y(ny) {}
    };

    struct shape {
        std::vector<v2f> vertices;
        std::vector<std::uint32_t> indices;

        std::vector<std::vector<v2f>> uvs_channels;
        std::vector<std::vector<std::uint32_t>> colors_channels;
    };

    template < typename T >
    T saturate(T v) noexcept {
        return std::min(std::max(v, T(0)), T(1));
    }

    std::uint8_t pack_color_component(float c) noexcept {
        return static_cast<std::uint8_t>(std::round(saturate(c) * 255.f));
    }

    std::uint32_t pack_color(float r, float g, float b, float a) noexcept {
        std::uint8_t rr = pack_color_component(r);
        std::uint8_t gg = pack_color_component(g);
        std::uint8_t bb = pack_color_component(b);
        std::uint8_t aa = pack_color_component(a);
        return
            static_cast<std::uint32_t>(aa) << 24 |
            static_cast<std::uint32_t>(rr) << 16 |
            static_cast<std::uint32_t>(gg) <<  8 |
            static_cast<std::uint32_t>(bb) <<  0;
    }

    void write_u32_to_ofstream(std::ofstream& s, const std::uint32_t v) {
        s.write(
            reinterpret_cast<const char*>(&v),
            sizeof(v));
    }

    void write_str_to_ofstream(std::ofstream& s, const std::string& v) {
        s.write(v.data(), static_cast<std::ptrdiff_t>(v.length()));
    }

    template < typename T >
    std::size_t write_vector_to_ofstream(std::ofstream& s, const std::vector<T>& v) {
        if ( !v.empty() ) {
            std::size_t data_size = v.size() * sizeof(T);
            s.write(
                reinterpret_cast<const char*>(v.data()),
                static_cast<std::ptrdiff_t>(data_size));
            return data_size;
        }
        return 0;
    }

    bool validate_shape(const shape& shape) noexcept {
        if ( shape.vertices.empty() ) {
            return false;
        }
        if ( shape.indices.empty() ) {
            return false;
        }
        for ( const auto& uvs : shape.uvs_channels ) {
            if ( uvs.size() != shape.vertices.size() ) {
                return false;
            }
        }
        for ( const auto& colors : shape.colors_channels ) {
            if ( colors.size() != shape.vertices.size() ) {
                return false;
            }
        }
        return true;
    }

    bool save_shape(const shape& shape, const std::string& out_path, const opts& opts) {
        timer save_timer;

        if ( !validate_shape(shape) ) {
            std::cerr << "Failed to validate out shape: " << out_path << std::endl;
            return false;
        }

        std::ofstream stream(out_path, std::ofstream::out | std::ofstream::binary);
        if ( !stream.is_open() ) {
            std::cerr << "Failed to open out file stream: " << out_path << std::endl;
            return false;
        }

        write_str_to_ofstream(stream, shape_file_signature);
        write_u32_to_ofstream(stream, shape_file_version);

        write_u32_to_ofstream(stream, static_cast<std::uint32_t>(shape.vertices.size()));
        write_u32_to_ofstream(stream, static_cast<std::uint32_t>(shape.indices.size()));

        write_u32_to_ofstream(stream, static_cast<std::uint32_t>(shape.uvs_channels.size()));
        write_u32_to_ofstream(stream, static_cast<std::uint32_t>(shape.colors_channels.size()));

        std::size_t vertices_bytes = write_vector_to_ofstream(stream, shape.vertices);
        std::size_t indices_bytes = write_vector_to_ofstream(stream, shape.indices);

        std::size_t uvs_bytes = 0;
        for ( const auto& uvs : shape.uvs_channels ) {
            uvs_bytes += write_vector_to_ofstream(stream, uvs);
        }

        std::size_t colors_bytes = 0;
        for ( const auto& colors : shape.colors_channels ) {
            colors_bytes += write_vector_to_ofstream(stream, colors);
        }

        if ( opts.timers ) {
            std::cout << "> save mesh: ";
            save_timer.done();
            std::cout << " - " << out_path << std::endl;
        }

        if ( opts.verbose ) {
            std::cout
                << std::endl
                << "> mesh info:" << std::endl
                << "-> vertices: " << shape.vertices.size() << ", " << vertices_bytes << " B" << std::endl
                << "-> indices: " << shape.indices.size() << ", " << indices_bytes << " B" << std::endl
                << "-> uvs: " << shape.uvs_channels.size() << ", " << uvs_bytes << " B" << std::endl
                << "-> colors: " << shape.colors_channels.size() << ", " << colors_bytes << " B" << std::endl;
        }

        return true;
    }

    bool convert_shape(const aiMesh* ai_mesh, const std::string& out_path, const opts& opts) {
        shape out_shape;
        timer convert_timer;

        if ( ai_mesh->HasPositions() ) {
            out_shape.vertices.reserve(ai_mesh->mNumVertices);
            std::transform(
                ai_mesh->mVertices,
                ai_mesh->mVertices + ai_mesh->mNumVertices,
                std::back_inserter(out_shape.vertices),
                [](const aiVector3D& v) noexcept {
                    return v2f{v.x, v.y};
                });
        }

        if ( ai_mesh->HasFaces() ) {
            out_shape.indices.reserve(ai_mesh->mNumFaces * 3u);
            std::for_each(
                ai_mesh->mFaces,
                ai_mesh->mFaces + ai_mesh->mNumFaces,
                [&out_shape](const aiFace& f) {
                    if ( f.mNumIndices != 3 ) {
                        throw std::logic_error("invalide face index count");
                    }
                    out_shape.indices.insert(
                        out_shape.indices.end(),
                        f.mIndices,
                        f.mIndices + f.mNumIndices);
                });
        }

        for ( unsigned int channel = 0; channel < ai_mesh->GetNumUVChannels(); ++channel ) {
            std::vector<v2f> uvs;
            uvs.reserve(ai_mesh->mNumVertices);
            std::transform(
                ai_mesh->mTextureCoords[channel],
                ai_mesh->mTextureCoords[channel] + ai_mesh->mNumVertices,
                std::back_inserter(uvs),
                [](const aiVector3D& v) noexcept {
                    return v2f{v.x, v.y};
                });
            out_shape.uvs_channels.emplace_back(std::move(uvs));
        }

        for ( unsigned int channel = 0; channel < ai_mesh->GetNumColorChannels(); ++channel ) {
            std::vector<std::uint32_t> colors;
            colors.reserve(ai_mesh->mNumVertices);
            std::transform(
                ai_mesh->mColors[channel],
                ai_mesh->mColors[channel] + ai_mesh->mNumVertices,
                std::back_inserter(colors),
                [](const aiColor4D& v) noexcept {
                    return pack_color(v.r, v.g, v.b, v.a);
                });
            out_shape.colors_channels.emplace_back(std::move(colors));
        }

        if ( opts.timers ) {
            std::cout << std::endl << "> convert shape: ";
            convert_timer.done();
            std::cout << " - " << out_path << std::endl;
        }

        return save_shape(out_shape, out_path, opts);
    }

    bool convert(const std::string& path, const opts& opts) {
        timer convert_timer;
        timer importer_timer;

        Assimp::Importer importer;

        if ( opts.timers ) {
            std::cout << "> prepare importer: ";
            importer_timer.done();
        }

        const unsigned int importer_flags =
            aiProcess_Triangulate |
            aiProcess_MakeLeftHanded |
            aiProcess_OptimizeMeshes |
            aiProcess_JoinIdenticalVertices;

        timer import_timer;

        const aiScene* scene = importer.ReadFile(path, importer_flags);
        if ( !scene ) {
            std::cerr << "Failed to import model: " << path << std::endl;
            std::cerr << "Error: " << importer.GetErrorString() << std::endl;
            return false;
        }

        if ( opts.timers ) {
            std::cout << "> import model: ";
            import_timer.done();
        }

        for ( unsigned int mesh_index = 0; mesh_index < scene->mNumMeshes; ++mesh_index ) {
            const aiMesh* mesh = scene->mMeshes[mesh_index];

            const std::string shape_name = mesh->mName.length
                ? mesh->mName.C_Str()
                : "shape_" + std::to_string(mesh_index);

            std::string shape_out_path = std::string()
                .append(path)
                .append(".")
                .append(shape_name)
                .append(".e2d_shape");

            if ( opts.verbose ) {
                std::cout
                    << std::endl
                    << ">> Shape("
                    << shape_name
                    << ") converting..."
                    << std::endl;
            }

            if ( !convert_shape(mesh, shape_out_path, opts) ) {
                std::cerr << "Failed!" << std::endl;
                return false;
            }

            if ( opts.verbose ) {
                std::cout << "OK. " << std::endl;
            }
        }

        if ( opts.timers ) {
            std::cout << std::endl << "=====" << std::endl;
            convert_timer.done();
        }

        return true;
    }
}

int main(int argc, char *argv[]) {
    if ( argc < 2 ) {
        std::cout << "USAGE: model2shape mesh.obj" << std::endl;
        return 0;
    }
    return convert(argv[1], opts(argc, argv)) ? 0 : 1;
}
