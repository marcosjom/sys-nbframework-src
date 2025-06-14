
#Configure
NB_CFG_PRINT_INTERNALS := 1
NB_CFG_PRINT_INFO      := 0

#Import functions
include MakefileFuncs.mk

#Init workspace
$(eval $(call nbCall,nbInitWorkspace))

#Project
include MakefileProject.mk

#Build workspace
$(eval $(call nbCall,nbBuildWorkspaceRules))
