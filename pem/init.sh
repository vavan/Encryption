#!/bin/bash

echo "Generate certificate request"
openssl req -new -nodes -out server.csr

echo "Generate private key"
openssl genrsa -out key.pem 2048

echo "Generate the certifucate"
openssl x509 -req -days 365 -in server.csr -signkey key.pem -out cert.pem

