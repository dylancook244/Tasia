# setup.py for development
from setuptools import setup, find_packages

setup(
    name="tasia",
    version="0.1.0",
    packages=find_packages(),
    include_package_data=True,
    package_data={
        'tasia': ['compiler/**/*'],  # Include all compiler files
    },
    entry_points={
        'console_scripts': [
            'tasia=tasia.tasia:main',
        ],
    },
)