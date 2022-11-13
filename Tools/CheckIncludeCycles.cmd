pushd header-cycle-checker
set RUSTFLAGS=-Awarnings
cargo build -q

cargo run -- ../../Source/SceneViewer -I../../Source/SceneViewer -I../../Source/Engine -I../../Source/Core -I../../Source/CLI 
pause