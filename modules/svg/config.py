def can_build(env, platform):
    env.module_add_dependencies("lottie", ["a_lz4"])
    return True


def configure(env):
    from SCons.Script import BoolVariable, Help, Variables

    env_vars = Variables()
    env_vars.Add(BoolVariable("lottie", "Enable Lottie support using thorvg", True))

    env_vars.Update(env)
    Help(env_vars.GenerateHelpText(env))
