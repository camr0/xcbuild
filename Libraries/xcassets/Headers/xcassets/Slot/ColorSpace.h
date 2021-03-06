/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#ifndef __xcassets_Slot_ColorSpace_h
#define __xcassets_Slot_ColorSpace_h

#include <ext/optional>
#include <string>

namespace xcassets {
namespace Slot {

/*
 * An image color space.
 */
enum class ColorSpace {
    /*
     * Standard color space.
     */
    sRGB,

    /*
     * Expanded color space.
     */
    DisplayP3,
};

class ColorSpaces {
private:
    ColorSpaces();
    ~ColorSpaces();

public:
    /*
     * Parse a matching compression from a string, if valid.
     */
    static ext::optional<ColorSpace> Parse(std::string const &value);

    /*
     * Convert a compression to a string.
     */
    static std::string String(ColorSpace compression);
};

}
}

#endif // !__xcassets_Slot_ColorSpace_h
