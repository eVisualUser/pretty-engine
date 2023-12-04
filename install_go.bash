
#!/bin/bash

OS="$(uname)"

if [ "$OS" = "Linux" ]; then
    echo "Installing Go for Linux..."

    wget https://golang.org/dl/go1.21.4.linux-amd64.tar.gz --no-check-certificate

    sudo tar -C /usr/local -xzf go1.21.4.linux-amd64.tar.gz

    echo "export PATH=$PATH:/usr/local/go/bin" >> ~/.bashrc

    source ~/.bashrc

elif [ "$OS" = "Darwin" ]; then
    echo "Installing Go for macOS..."

    curl -O https://golang.org/dl/go1.21.4.darwin-amd64.tar.gz

    sudo tar -C /usr/local -xzf go1.21.4.darwin-amd64.tar.gz

    echo "export PATH=$PATH:/usr/local/go/bin" >> ~/.bash_profile

    source ~/.bash_profile

else
    echo "Unsupported operating system."
    exit 1
fi

echo "Go installation completed successfully!"
