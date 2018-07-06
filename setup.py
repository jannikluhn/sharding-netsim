from setuptools import setup, find_packages

setup(
    name="sharding-netsim",
    version="0.0.1",
    description="Ethereum sharding network simulations",
    url="http://github.com/jannikluhn/sharding-netsim",
    author="Jannik Luhn",
    author_email="jannik@brainbot.com",
    license="MIT",
    packages=find_packages(exclude=["tests", "tests.*"]),
    zip_safe=False,
)
