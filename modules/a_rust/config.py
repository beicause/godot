def can_build(env,platform):
    env.module_add_dependencies("a_rust", ["a_lz4","a_jsonnet"])
    return env["platform"] in ["android","linuxbsd"] and env["arch"] in ["arm64","x86_64"]

def configure(env):
    pass

def get_doc_classes():
    return [
        "Glicol",
        "JMESExpr",
        "JMESVariable",
        "JSONConverter",
    ]

def get_doc_path():
    return "doc_classes"
