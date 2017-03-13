/* 
 *  Copyright (c) 2008 Texas Instruments and others.
 *  All rights reserved. This program and the accompanying materials
 *  are made available under the terms of the Eclipse Public License v1.0
 *  which accompanies this distribution, and is available at
 *  http://www.eclipse.org/legal/epl-v10.html
 * 
 *  Contributors:
 *      Texas Instruments - initial implementation
 * 
 * */

var M3;

/*
 *  ======== M3.getISAChain ========
 *  M3 implementation for ITarget.getISAChain()
 */
function getISAChain (isa)
{
    var myChain = [this.isa];
    var isaIn = (isa == null ? this.isa : isa)

    for (var i = 0; i < myChain.length; i++) {
        if (myChain[i] == isaIn) {
            break;
        }
    }

    if (i == myChain.length) {
        return (null);
    }
    else {
        return (myChain.slice(0, i + 1));
    }
}

/*
 *  ======== module$meta$init ========
 */
function module$meta$init()
{
    M3 = this;
}

/*
 *  ======== M3.compile ========
 */
function compile(goal) {
    if (M3.targetPkgPath == null) {
        M3.targetPkgPath = this.$package.packageBase;
    }

    goal.opts.copts += " -I" + M3.targetPkgPath +
        "/libs/install-native/$(GCCTARG)/include ";

    goal.opts.cfgcopts += " -I" + M3.targetPkgPath +
        "/libs/install-native/$(GCCTARG)/include ";

    return (this.$super.compile(goal));
}

/*
 *  ======== M3.link ========
 */
function link(goal)
{
    if (M3.targetPkgPath == null) {
        M3.targetPkgPath = this.$package.packageBase;
    }

    goal.opts += " -L" + M3.targetPkgPath +
        "/libs/install-native/$(GCCTARG)/lib/armv7-m ";

    return(this.$super.link(goal));
}
/*
 *  @(#) gnu.targets.arm; 1, 0, 0,0; 2-10-2017 09:06:33; /db/ztree/library/trees/xdctargets/xdctargets-m11/src/ xlibrary

 */

