from setuptools import setup, Extension

# Define the extension module
extension_mod = Extension("simple_graphs",
                           sources=["simple_graphs.c"])

# Setup
setup(name="simple_graphs",
      version="1.0",
      description="A simple graph operations module",
      ext_modules=[extension_mod])
