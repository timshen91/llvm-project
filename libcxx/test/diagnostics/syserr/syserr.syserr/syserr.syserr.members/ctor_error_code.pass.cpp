//===----------------------------------------------------------------------===//
//
// ΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚThe LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <system_error>

// class system_error

// system_error(error_code ec);

// Test is slightly non-portable

#include <system_error>
#include <string>
#include <cassert>

int main()
{
    std::system_error se(static_cast<int>(std::errc::not_a_directory),
                         std::generic_category(), "some text");
    assert(se.code() == std::make_error_code(std::errc::not_a_directory));
    std::string what_message(se.what());
    assert(what_message.find("Not a directory") != std::string::npos);
}
