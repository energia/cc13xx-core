/* 
 *  Copyright (c) 2015 Texas Instruments Incorporated
 *  All rights reserved. This program and the accompanying materials
 *  are made available under the terms of the Eclipse Public License v1.0
 *  which accompanies this distribution, and is available at
 *  http://www.eclipse.org/legal/epl-v10.html
 *
 *  Contributors:
 *      Texas Instruments - initial implementation
 *
 * */

/*!
 *  ======== IR4.xdc ========
 *  Cortex R4 little endian target
 */
metaonly interface IR4 inherits ti.targets.arm.elf.IArm {
    override readonly config string isa         = "v7R";

    override config string platform   = "ti.platforms.evmTMS570";

    override readonly config xdc.bld.ITarget2.Command lnk = {
        cmd:  "armlnk",
        opts: "--silicon_version=7R4 --strict_compatibility=on"
    };
}
/*
 *  @(#) ti.targets.arm.elf; 1, 0, 0,0; 2-10-2017 09:06:44; /db/ztree/library/trees/xdctargets/xdctargets-m11/src/ xlibrary

 */

