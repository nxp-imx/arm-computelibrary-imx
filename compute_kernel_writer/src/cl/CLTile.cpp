/*
 * Copyright (c) 2023 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "ckw/Error.h"
#include "ckw/TileInfo.h"

#include "src/Helpers.h"
#include "src/cl/CLHelpers.h"
#include "src/cl/CLTile.h"

#include <algorithm>
#include <vector>

namespace ckw
{
CLTile::CLTile(const std::string &name, const TileInfo &info)
{
    validate_tile_info(info);

    _basename = name;
    _info     = info;
}

TileVariable CLTile::scalar(int32_t row, int32_t col) const
{
    // Clamp to nearest valid edge
    col = clamp(col, static_cast<int32_t>(0), _info.width() - 1);
    row = clamp(row, static_cast<int32_t>(0), _info.height() - 1);

    TileVariable t;
    t.str      = create_var_name(row);
    t.desc.dt  = _info.data_type();
    t.desc.len = 1;

    // This check is required because if the width has only one element, we cannot use .s0
    if(_info.width() != 1)
    {
        // Automatic broadcasting
        t.str += ".s" + dec_to_hex_as_string(col);
    }

    return t;
}

TileVariable CLTile::vector(int32_t row) const
{
    // Clamp to nearest valid edge
    row = clamp(row, static_cast<int32_t>(0), _info.height() - 1);

    TileVariable t;
    t.str      = create_var_name(row);
    t.desc.dt  = _info.data_type();
    t.desc.len = _info.width();
    return t;
}

TileVariable CLTile::vector(int32_t row, int32_t col_start, int32_t width) const
{
    // Validate the new vector length
    cl_validate_vector_length(width);

    // Clamp to nearest valid edge
    row = clamp(row, static_cast<int32_t>(0), _info.height() - 1);

    TileVariable t;
    t.str      = create_var_name(row);
    t.desc.dt  = _info.data_type();
    t.desc.len = width;

    if(_info.width() != 1)
    {
        t.str += ".s";
        for(int i = 0; i < width; ++i)
        {
            t.str += dec_to_hex_as_string(col_start + i);
        }
    }
    return t;
}

std::vector<TileVariable> CLTile::all() const
{
    std::vector<TileVariable> vars;
    for(int32_t y = 0; y < _info.height(); ++y)
    {
        TileVariable t;
        t.str      = create_var_name(y);
        t.desc.dt  = _info.data_type();
        t.desc.len = _info.width();
        vars.push_back(t);
    }
    return vars;
}

bool CLTile::is_assignable() const
{
    return true;
}

std::string CLTile::create_var_name(int32_t row) const
{
    std::string var_name = _basename;

    // If a scalar variable, we do not append the row index
    if(_info.height() == 1)
    {
        return var_name;
    }
    else
    {
        var_name += "_";
        var_name += std::to_string(row);
    }

    return var_name;
}
} // namespace ckw