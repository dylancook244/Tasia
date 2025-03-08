make install &&
tasia run hello.sia

to check version (rust example):
curl -s "https://api.github.com/repos/rust-lang/rust/releases/latest" | grep '"tag_name":' | cut -d'"' -f4
