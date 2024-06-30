# editor

## neovim@0.10.0
curl -sL https://github.com/neovim/neovim/releases/download/v0.10.0/nvim-linux64.tar.gz -o /tmp/nvim-linux64.tar.gz
tar -xvf /tmp/nvim-linux64.tar.gz --directory=/tmp
sudo cp /tmp/nvim-linux64/bin/nvim /usr/bin
sudo cp -r /tmp/nvim-linux64/share/nvim /usr/share
rm /tmp/nvim-linux64.tar.gz
rm -r /tmp/nvim-linux64

## astronvim
git clone --depth 1 https://github.com/AstroNvim/template ${HOME}/.config/nvim
rm -rf ${HOME}/.config/nvim/.git
