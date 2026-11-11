use std::env;
use std::fs;
use std::path::{Path, PathBuf};

fn main() {
    let ruta_essp = Path::new("../eSSP/lib/Encryption.c");

    if ruta_essp.exists() {
        cc::Build::new()
            .include("../eSSP/inc")
            .include("../eSSP/lib")
            .file("../eSSP/lib/Encryption.c")
            .file("../eSSP/lib/ITLSSPProc.c")
            .file("../eSSP/lib/Random.c")
            .file("../eSSP/lib/SSPComs.c")
            .file("../eSSP/lib/SSPDownload.c")
            .file("../eSSP/lib/serialfunc.c")
            .file("../eSSP/lib/ssp_commands.c")
            .warnings(false)
            .compile("itl_essp_c");
    }

    cxx_build::bridge("src/lib.rs")
        .flag_if_supported("-std=c++20")
        .flag_if_supported("-Wno-maybe-uninitialized")
        .compile("essp_driver_bridge");

    // Exportar siempre las cabeceras actualizadas a lib/essp_driver/generated/
    if let Ok(out_dir) = env::var("OUT_DIR") {
        let out_inc = PathBuf::from(out_dir).join("cxxbridge").join("include");
        let gen_dir = Path::new("generated");

        let _ = fs::create_dir_all(gen_dir.join("essp_driver/src"));
        let _ = fs::create_dir_all(gen_dir.join("rust"));

        let _ = fs::copy(
            out_inc.join("essp_driver/src/lib.rs.h"),
            gen_dir.join("essp_driver/src/lib.rs.h"),
        );
        let _ = fs::copy(
            out_inc.join("rust/cxx.h"),
            gen_dir.join("rust/cxx.h"),
        );
    }

    println!("cargo:rerun-if-changed=src/lib.rs");
}