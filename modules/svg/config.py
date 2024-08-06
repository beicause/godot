def can_build(env, platform):
    env.module_add_dependencies("lottie", ["a_lz4"])
    return True


def configure(env):
    pass
