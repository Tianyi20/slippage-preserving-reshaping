from setuptools import setup, find_packages

try:
    from wheel.bdist_wheel import bdist_wheel as _bdist_wheel

    class bdist_wheel(_bdist_wheel):
        def finalize_options(self):
            super().finalize_options()
            self.root_is_pure = False

    cmdclass = {"bdist_wheel": bdist_wheel}
except Exception:
    cmdclass = {}


setup(
    packages=find_packages(),
    package_data={"slippage_reshaping": ["*.so"]},
    include_package_data=True,
    zip_safe=False,
    cmdclass=cmdclass,
)
