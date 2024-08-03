use jmespath;

pub struct JMESExpr {
    expr: Option<jmespath::Expression<'static>>,
}

pub struct JMESVariable {
    var: jmespath::Variable,
}

impl JMESExpr {
    fn create_from_str(str: &str) -> Self {
        let expr = jmespath::compile(str);
        if expr.is_err() {
            println!("JMESPath expr compilation error: {}", expr.unwrap_err());
            Self { expr: None }
        } else {
            Self {
                expr: Some(expr.unwrap()),
            }
        }
    }

    fn to_str(&self) -> String {
        if let Some(e) = &self.expr {
            return e.as_str().to_string()
        }
        "".to_string()
    }

    fn search(&self, data: &JMESVariable) -> String {
        if self.expr.is_none() {
            return "".to_string();
        }
        match self.expr.as_ref().unwrap().search(&data.var) {
            Ok(v) => serde_json::to_string(&v).unwrap(),
            Err(err) => {
                println!("JMESPath search error: {}", err);
                "".to_string()
            }
        }
    }
}

impl JMESVariable {
    fn create_from_json_str(str: &str) -> Self {
        let var = jmespath::Variable::from_json(str);
        if var.is_err() {
            println!(
                "JMESPath variable deserialization error: {}",
                var.unwrap_err()
            );
            Self {
                var: jmespath::Variable::Null,
            }
        } else {
            Self { var: var.unwrap() }
        }
    }

    fn to_json_str(&self, pretty: bool) -> String {
        if pretty {
            serde_json::to_string_pretty(&self.var).unwrap()
        } else {
            serde_json::to_string(&self.var).unwrap()
        }
    }
}

pub fn jmesexpr_create_from_str(str: &str) -> Box<JMESExpr> {
    Box::new(JMESExpr::create_from_str(str))
}
pub fn jmesvariable_create_from_json_str(str: &str) -> Box<JMESVariable> {
    Box::new(JMESVariable::create_from_json_str(str))
}

#[cxx::bridge(namespace = "jmespath")]
pub mod ffi {
    extern "Rust" {
        type JMESExpr;
        type JMESVariable;
        fn jmesexpr_create_from_str(str: &str) -> Box<JMESExpr>;
        fn jmesvariable_create_from_json_str(str: &str) -> Box<JMESVariable>;
        fn to_str(self: &JMESExpr) -> String;
        fn search(self: &JMESExpr, data: &JMESVariable) -> String;
        fn to_json_str(self: &JMESVariable, pretty: bool) -> String;
    }
}
