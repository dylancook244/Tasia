from setuptools import setup, find_packages

setup(
    name="tasia",
    version="0.1.0",
    packages=find_packages(),
    py_modules=["tasia"],
    entry_points={
        'console_scripts': [
            'tasia=tasia:main',  # Points to the main() function in tasia.py
        ],
    },
    author="Dylan Cook",
    description="The Tasia Programming Language CLI",
    # Add other metadata as needed
)