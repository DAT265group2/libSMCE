/*
 *  test/BoardView.cpp
 *  Copyright 2020-2021 ItJustWorksTM
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include <array>
#include <chrono>
#include <iostream>
#include <thread>
#include <catch2/catch_test_macros.hpp>
#include "SMCE/Board.hpp"
#include "SMCE/BoardConf.hpp"
#include "SMCE/BoardView.hpp"
#include "SMCE/Toolchain.hpp"
#include "defs.hpp"

using namespace std::literals;

constexpr std::byte operator""_b(char c) noexcept { return static_cast<std::byte>(c); }
constexpr std::byte operator""_b(unsigned long long c) noexcept { return static_cast<std::byte>(c); }


TEST_CASE("BoardView GPIO", "[BoardView]") {
    smce::Toolchain tc{SMCE_PATH};
    REQUIRE(!tc.check_suitable_environment());
    smce::Sketch sk{SKETCHES_PATH "pins", {.fqbn = "arduino:avr:nano"}};
    const auto ec = tc.compile(sk);
    if (ec)
        std::cerr << tc.build_log().second;
    REQUIRE_FALSE(ec);
    smce::Board br{};
    // clang-format off
    smce::BoardConfig bc{
        /* .pins = */{0, 2},
        /* .gpio_drivers = */{
            smce::BoardConfig::GpioDrivers{
                0,
                smce::BoardConfig::GpioDrivers::DigitalDriver{true, false},
                smce::BoardConfig::GpioDrivers::AnalogDriver{true, false}
            },
            smce::BoardConfig::GpioDrivers{
                2,
                smce::BoardConfig::GpioDrivers::DigitalDriver{false, true},
                smce::BoardConfig::GpioDrivers::AnalogDriver{false, true}
            },
        }
    };
    // clang-format on
    REQUIRE(br.configure(std::move(bc)));
    REQUIRE(br.attach_sketch(sk));
    REQUIRE(br.start());
    auto bv = br.view();
    REQUIRE(bv.valid());
    auto pin0 = bv.pins[0];
    REQUIRE(pin0.exists());
    auto pin0d = pin0.digital();
    REQUIRE(pin0d.exists());
    REQUIRE(pin0d.can_read());
    REQUIRE_FALSE(pin0d.can_write());
    auto pin0a = pin0.analog();
    REQUIRE(pin0a.exists());
    REQUIRE(pin0a.can_read());
    REQUIRE_FALSE(pin0a.can_write());
    auto pin1 = bv.pins[1];
    REQUIRE_FALSE(pin1.exists());
    auto pin2 = bv.pins[2];
    REQUIRE(pin2.exists());
    auto pin2d = pin2.digital();
    REQUIRE(pin2d.exists());
    REQUIRE_FALSE(pin2d.can_read());
    REQUIRE(pin2d.can_write());
    auto pin2a = pin2.analog();
    REQUIRE(pin2a.exists());
    REQUIRE_FALSE(pin2a.can_read());
    REQUIRE(pin2a.can_write());
    std::this_thread::sleep_for(1ms);

    pin0d.write(false);
    test_pin_delayable(pin2d, true, 16384, 1ms);
    pin0d.write(true);
    test_pin_delayable(pin2d, false, 16384, 1ms);
    REQUIRE(br.stop());
}

TEST_CASE("BoardView UART", "[BoardView]") {
    smce::Toolchain tc{SMCE_PATH};
    REQUIRE(!tc.check_suitable_environment());
    smce::Sketch sk{SKETCHES_PATH "uart", {.fqbn = "arduino:avr:nano"}};
    const auto ec = tc.compile(sk);
    if (ec)
        std::cerr << tc.build_log().second;
    REQUIRE_FALSE(ec);
    smce::Board br{};
    REQUIRE(br.configure({.uart_channels = {{}}}));
    REQUIRE(br.attach_sketch(sk));
    REQUIRE(br.start());
    auto bv = br.view();
    REQUIRE(bv.valid());
    auto uart0 = bv.uart_channels[0];
    REQUIRE(uart0.exists());
    REQUIRE(uart0.rx().exists());
    REQUIRE(uart0.tx().exists());
    auto uart1 = bv.uart_channels[1];
    REQUIRE_FALSE(uart1.exists());
    REQUIRE_FALSE(uart1.rx().exists());
    REQUIRE_FALSE(uart1.tx().exists());
    std::this_thread::sleep_for(1ms);

    std::array out = {'H', 'E', 'L', 'L', 'O', ' ', 'U', 'A', 'R', 'T', '\0'};
    std::array<char, out.size()> in{};
    REQUIRE(uart0.rx().write(out) == out.size());
    int ticks = 16'000;
    do {
        if (ticks-- == 0)
            FAIL("Timed out");
        std::this_thread::sleep_for(1ms);
    } while (uart0.tx().size() != in.size());
    REQUIRE(uart0.tx().front() == 'H');
    REQUIRE(uart0.tx().read(in) == in.size());
    REQUIRE(uart0.tx().front() == '\0');
    REQUIRE(uart0.tx().size() == 0);
    REQUIRE(in == out);

#if !MSVC_DEBUG
    std::reverse(out.begin(), out.end());
    REQUIRE(uart0.rx().write(out) == out.size());
    ticks = 16'000;
    do {
        if (ticks-- == 0)
            FAIL("Timed out");
        std::this_thread::sleep_for(1ms);
    } while (uart0.tx().size() != in.size());
    REQUIRE(uart0.tx().read(in) == in.size());
    REQUIRE(uart0.tx().size() == 0);
    REQUIRE(in == out);
#endif

    REQUIRE(br.stop());
}

constexpr auto div_ceil(std::size_t lhs, std::size_t rhs) { return lhs / rhs + !!(lhs % rhs); }

constexpr std::size_t bpp_444 = 4 + 4 + 4;
constexpr std::size_t bpp_888 = 8 + 8 + 8;

TEST_CASE("BoardView RGB444 cvt", "[BoardView]") {
    smce::Toolchain tc{SMCE_PATH};
    REQUIRE(!tc.check_suitable_environment());
    smce::Sketch sk{SKETCHES_PATH "noop", {.fqbn = "arduino:avr:nano"}};
    const auto ec = tc.compile(sk);
    if (ec)
        std::cerr << tc.build_log().second;
    REQUIRE_FALSE(ec);
    smce::Board br{};
    REQUIRE(br.configure({.frame_buffers = {{}}}));
    REQUIRE(br.attach_sketch(sk));
    REQUIRE(br.prepare());
    auto bv = br.view();
    REQUIRE(bv.valid());
    REQUIRE(br.start());
    REQUIRE(br.suspend());
    auto fb = bv.frame_buffers[0];
    REQUIRE(fb.exists());

    {
        constexpr std::size_t height = 1;
        constexpr std::size_t width = 1;

        constexpr std::array in = {'\xBC'_b, '\x0A'_b};
        constexpr std::array expected_out = {'\xA0'_b, '\xB0'_b, '\xC0'_b};
        static_assert(in.size() == expected_out.size() / 3 * 2);

        fb.set_height(height);
        fb.set_width(width);
        REQUIRE(fb.write_rgb444(in));

        std::array<std::byte, std::size(expected_out)> out;
        REQUIRE(fb.read_rgb888(out));
        REQUIRE(out == expected_out);
    }

    {
        constexpr std::size_t height = 2;
        constexpr std::size_t width = 2;

        constexpr std::array in = {'\x23'_b, '\xF1'_b, '\x56'_b, '\xF4'_b, '\x89'_b, '\xF7'_b, '\xBC'_b, '\xFA'_b};
        constexpr std::array expected_out = {'\x10'_b, '\x20'_b, '\x30'_b, '\x40'_b, '\x50'_b, '\x60'_b,
                                             '\x70'_b, '\x80'_b, '\x90'_b, '\xA0'_b, '\xB0'_b, '\xC0'_b};
        static_assert(in.size() == expected_out.size() / 3 * 2);

        fb.set_height(height);
        fb.set_width(width);
        fb.write_rgb444(in);

        std::array<std::byte, std::size(expected_out)> out;
        fb.read_rgb888(out);
        REQUIRE(out == expected_out);
    }

    {
        constexpr std::size_t height = 1;
        constexpr std::size_t width = 1;

        constexpr std::array in = {'\xAD'_b, '\xBE'_b, '\xCF'_b};
        constexpr std::array expected_out = {'\xBC'_b, '\x0A'_b};
        static_assert(expected_out.size() == in.size() / 3 * 2);

        fb.set_height(height);
        fb.set_width(width);
        REQUIRE(fb.write_rgb888(in));

        std::array<std::byte, std::size(expected_out)> out;
        REQUIRE(fb.read_rgb444(out));
        REQUIRE(out == expected_out);
    }

    {
        constexpr std::size_t height = 2;
        constexpr std::size_t width = 2;

        constexpr std::array in = {'\x1A'_b, '\x2B'_b, '\x3C'_b, '\x4D'_b, '\x5E'_b, '\x6F'_b,
                                   '\x7A'_b, '\x8B'_b, '\x9C'_b, '\xAD'_b, '\xBE'_b, '\xCF'_b};
        constexpr std::array expected_out = {'\x23'_b, '\x01'_b, '\x56'_b, '\x04'_b,
                                             '\x89'_b, '\x07'_b, '\xBC'_b, '\x0A'_b};
        static_assert(expected_out.size() == in.size() / 3 * 2);

        fb.set_height(height);
        fb.set_width(width);
        fb.write_rgb888(in);

        std::array<std::byte, std::size(expected_out)> out;
        fb.read_rgb444(out);
        REQUIRE(out == expected_out);
    }

    REQUIRE(br.resume());
    REQUIRE(br.stop());
}

/*
 * rgb888ToRgb565
 * 76543210 | 76543210
 * GGGBBBBB   RRRRRGGG
 */
TEST_CASE("BoardView RGB565 Read Test", "[BoardView]") {
    // convert three bytes(24bits) of rgb888 to two bytes(18bits) of rgb565
    std::byte res[2];
    std::byte red_bits[] = {0xFF_b, 0_b, 0_b};
    std::byte green_bits[] = {0_b, 0xFF_b, 0_b};
    std::byte blue_bits[] = {0x0_b, 0_b, 0xFF_b};
    std::byte black_bits[] = {0_b, 0_b, 0_b};
    std::byte white_bits[] = {0xFF_b, 0xFF_b, 0xFF_b};
    std::byte purple_bits[] = {0xa4_b, 0x52_b, 0xcd_b};

    smce::rgb888ToRgb565(red_bits, res);
    // red
    REQUIRE(res[1] == 0xF8_b);
    // green and blue
    REQUIRE(res[0] == 0_b);

    smce::rgb888ToRgb565(green_bits, res);
    REQUIRE(res[1] == 0x07_b);
    REQUIRE(res[0] == 0xE0_b);

    smce::rgb888ToRgb565(blue_bits, res);
    REQUIRE(res[1] == 0_b);
    REQUIRE(res[0] == 0x1F_b);

    smce::rgb888ToRgb565(black_bits, res);
    REQUIRE(res[1] == 0_b);
    REQUIRE(res[0] == 0_b);

    smce::rgb888ToRgb565(white_bits, res);
    REQUIRE(res[1] == 0xFF_b);
    REQUIRE(res[0] == 0xFF_b);

    smce::rgb888ToRgb565(purple_bits, res);
    REQUIRE(res[1] == 0xA_b);
    REQUIRE(res[0] == 0x5C_b);
}

TEST_CASE("BoardView RGB565 Write Test", "[BoardView]") {
    // convert two bytes(16bits) of rgb565  to three bytes(24bits) of rgb888
    std::byte res[3];
    std::byte red_bits[] = {0x00_b, 0xF8_b};
    std::byte green_bits[] = {0xE0_b, 0x07_b};
    std::byte blue_bits[] = {0x1F_b, 0_b};
    std::byte black_bits[] = {0_b, 0_b};
    std::byte white_bits[] = {0xFF_b, 0xFF_b};
    std::byte purple_bits[] = {0x99_b, 0xA2_b};

    smce::rgb565ToRgb888(std::span{red_bits, 2}, res);
    // red
    REQUIRE(res[0] == 255_b);
    // green
    REQUIRE(res[1] == 0_b);
    // blue
    REQUIRE(res[2] == 0_b);

    smce::rgb565ToRgb888(std::span{green_bits, 2}, res);
    REQUIRE(res[0] == 0_b);
    REQUIRE(res[1] == 255_b);
    REQUIRE(res[2] == 0_b);

    smce::rgb565ToRgb888(std::span{blue_bits, 2}, res);
    REQUIRE(res[0] == 0_b);
    REQUIRE(res[1] == 0_b);
    REQUIRE(res[2] == 255_b);

    smce::rgb565ToRgb888(std::span{black_bits, 2}, res);
    REQUIRE(res[0] == 0_b);
    REQUIRE(res[1] == 0_b);
    REQUIRE(res[2] == 0_b);

    smce::rgb565ToRgb888(std::span{white_bits, 2}, res);
    REQUIRE(res[0] == 255_b);
    REQUIRE(res[1] == 255_b);
    REQUIRE(res[2] == 255_b);

    smce::rgb565ToRgb888(std::span{purple_bits, 2}, res);
    REQUIRE(res[0] == 0xA5_b);
    REQUIRE(res[1] == 0x51_b);
    REQUIRE(res[2] == 0xCE_b);
}


