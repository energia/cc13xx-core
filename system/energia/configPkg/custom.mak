## THIS IS A GENERATED FILE -- DO NOT EDIT
.configuro: .libraries,m3g linker.cmd package/cfg/energia_pm3g.om3g

# To simplify configuro usage in makefiles:
#     o create a generic linker command file name 
#     o set modification times of compiler.opt* files to be greater than
#       or equal to the generated config header
#
linker.cmd: package/cfg/energia_pm3g.xdl
	$(SED) 's"^\"\(package/cfg/energia_pm3gcfg.cmd\)\"$""\"/db/ztree/library/trees/emt/emt-d09/src/bundles/energia/cc13xx/configPkg/\1\""' package/cfg/energia_pm3g.xdl > $@
	-$(SETDATE) -r:max package/cfg/energia_pm3g.h compiler.opt compiler.opt.defs
