{
  description = "A very basic flake";

  outputs = { self, nixpkgs }: let 
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in {
    packages.x86_64-linux.glPray = pkgs.stdenv.mkDerivation {
      name ="glPrey";
      src = self;
      
      nativeBuildInputs = [
        pkgs.pkg-config
      ];
      
      buildInputs = [
        pkgs.SDL2
        pkgs.libGLU
      ];

      makeFlags = [
        "DESTDIR=${placeholder "out"}"
      ];
    };

    packages.x86_64-linux.default = self.packages.x86_64-linux.glPray;

  };
}
