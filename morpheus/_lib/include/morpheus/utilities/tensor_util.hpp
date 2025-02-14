/**
 * SPDX-FileCopyrightText: Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "morpheus/objects/tensor_object.hpp"

#include <iosfwd>  // for ostream
#include <string>  // for string
#include <vector>  // for vector

namespace morpheus {

/**
 * @brief Tensor Utilities
 *
 * @note A tensor whose values are laid out in the storage starting from the rightmost
 * dimension onward (that is, moving along rows for a 2D tensor) is defined as contiguous.
 */
struct TensorUtils
{
    using shape_type = std::vector<TensorIndex>;

    /**
     * @brief Write a formatted shape to a stream
     *
     * @param shape
     * @param os
     */
    static void write_shape_to_stream(const shape_type& shape, std::ostream& os);

    /**
     * @brief Convenience method to get a string from write_shape_to_stream
     *
     * @param shape
     * @return std::string
     */
    static std::string shape_to_string(const shape_type& shape);

    /**
     * @brief Set stride to be contiguous with respect to row-major layouts
     *
     * @param shape
     * @param stride
     */
    static void set_contiguous_stride(const std::vector<TensorIndex>& shape, std::vector<TensorIndex>& stride);

    /**
     * @brief Determines if the tensor layout is both contiguous and ordered.
     *
     * @note A tensor whose values are laid out in the storage starting from the rightmost
     * dimension onward (that is, moving along rows for a 2D tensor) is defined as contiguous.
     */
    static bool has_contiguous_stride(const std::vector<TensorIndex>& shape, const shape_type& stride);

    /**
     * @brief Validate the shape and stride are compatible
     *
     * @param shape
     * @param stride
     * @return true
     * @return false
     */
    static bool validate_shape_and_stride(const std::vector<TensorIndex>& shape,
                                          const std::vector<TensorIndex>& stride);
};

}  // namespace morpheus
