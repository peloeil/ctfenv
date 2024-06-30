#!/bin/sh

if [ -e .env ]; then
    rm .env
fi

echo "UID=$(id -u $USER)" >> .env
echo "GID=$(id -g $USER)" >> .env
echo "USERNAME=$USER" >> .env
echo "GROUPNAME=$USER" >> .env
