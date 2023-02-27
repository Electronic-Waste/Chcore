#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import re

LICENSE = """\
/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v1. You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PSL v1 for more details.
 */
"""

for root, dirs, files in os.walk(sys.argv[1]):
    for file in files:
        if re.search(r"\.(c|C|cpp|cxx|cc|h|H|hpp|hxx|hh|s|S|asm|ASM)$", file):
            print(os.path.join(root, file))
            with open(os.path.join(root, file), "r") as f:
                content = f.readlines()
                for line in content[:10]:
                    if "license" in line.lower() or "copyright" in line.lower():
                        break
                else:
                    print("add license to {}".format(os.path.join(root, file)))
                    with open(os.path.join(root, file), "w") as f:
                        f.write(LICENSE + "\n")
                        f.writelines(content)
