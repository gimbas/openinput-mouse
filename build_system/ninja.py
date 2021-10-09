# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2021 Filipe Laíns <lains@riseup.net>

from __future__ import annotations

import contextlib
import os
import pathlib
import typing

from typing import Any, Dict, Iterator, List, Union

import ninja_syntax


if typing.TYPE_CHECKING:
    from . import BuildDetails, BuildLocation, BuildSettings, TargetInfo
    from .dependencies import Dependency


class NinjaError(Exception):
    pass


class NinjaBuilder:
    def __init__(
        self,
        writer: ninja_syntax.Writer,
        location: BuildLocation,
        settings: BuildSettings,
        target: TargetInfo,
    ) -> None:
        self.writer = writer
        self._location = location
        self._settings = settings
        self._target = target

        self._path_placeholders = {
            'root': location.source,
            'builddir': location.build,
        }

    @classmethod
    @contextlib.contextmanager
    def from_path(
        cls,
        path: Union[str, os.PathLike[str]],
        location: BuildLocation,
        settings: BuildSettings,
        target: TargetInfo,
    ) -> Iterator[NinjaBuilder]:
        with open(path, 'w') as f:
            yield cls(ninja_syntax.Writer(f, width=160), location, settings, target)

    def tool_name(self, name: str) -> str:
        return '-'.join(filter(None, [self._settings.toolchain, name]))

    def path(self, path: pathlib.Path) -> str:
        """Path representation for the build system - path using the $root and $builddir placeholders."""
        path = path.absolute()
        for placeholder, placeholder_path in self._path_placeholders.items():
            try:
                return os.path.join(f'${placeholder}', path.relative_to(placeholder_path.absolute()))
            except ValueError:
                pass
        return str(path)

    def remove_ext(self, file: pathlib.Path) -> pathlib.Path:
        return file.parent / file.stem

    def built(self, file: pathlib.Path) -> pathlib.Path:
        # is relative to source, so we calculate its path in outdir
        if file.is_relative_to(self._location.source):
            return self._location.build / self._target.name / file.relative_to(self._location.source)
        # otherwise is relative and we just append it to the outdir location
        if file.is_absolute():
            raise NinjaError(f'File is not relative to source so it can\'t be absolute: {file}')
        return self._location.build / self._target.name / file

    def cc(self, file: pathlib.Path, **kwargs: Any) -> List[str]:
        out_file = file.parent / (file.stem + '.o')
        return self.writer.build(  # type: ignore
            self.path(self.built(out_file)),
            'cc',
            self.path(file),
            **kwargs,
        )

    def build(self, file: pathlib.Path, rule: str, sources: List[str], **kwargs: Any) -> List[str]:
        return self.writer.build(  # type: ignore
            self.path(self.built(file)), rule, sources, **kwargs
        )

    def write_header(self, generator: str) -> None:
        self.writer.comment(f'build file automatically generated by {generator}, do not edit manually!')
        self.writer.newline()
        self.writer.variable('ninja_required_version', '1.3')
        self.writer.newline()

    def write_variables(
        self,
        details: BuildDetails,
        settings: BuildSettings,
        dependencies: Dict[str, Dependency],
    ) -> None:
        self.writer.comment('variables')
        self.writer.newline()

        # paths
        for placeholder, path in self._path_placeholders.items():
            self.writer.variable(placeholder, path)

        # tools
        self.writer.variable('cc', self.tool_name(settings.compiler))
        self.writer.variable('ar', self.tool_name('ar'))
        self.writer.variable('objcopy', self.tool_name('objcopy'))
        self.writer.variable('size', self.tool_name('size'))

        # flags
        self.writer.variable('c_flags', details.c_flags)
        self.writer.variable('c_include_flags', [
            f'-I{self.path(self._location.code)}'
        ] + [
            f'-include {self.path(path)}' for path in details.include_files
        ] + [
            f'-I{self.path(path)}'
            for dep in dependencies.values()
            for path in dep.external_include
        ])
        self.writer.variable('ld_flags', details.ld_flags + ([
            f'-L{self.path(details.linker_dir)}',
            f'-T{details.linker.relative_to(details.linker_dir)}',
        ] if details.linker else []))

        self.writer.newline()

    def write_rules(self) -> None:
        self.writer.comment('rules')
        self.writer.newline()
        self.writer.rule(
            'cc',
            command='$cc -MMD -MT $out -MF $out.d $c_flags $c_include_flags -c $in -o $out',
            depfile='$out.d',
            deps='gcc',
            description='CC $out',
        )
        self.writer.newline()
        self.writer.rule(
            'ar',
            command='rm -f $out && $ar crs $out $in',
            description='AR $out',
        )
        self.writer.newline()
        self.writer.rule(
            'link',
            command='$cc -o $out $in $ld_flags',
            description='LINK $out',
        )
        self.writer.newline()
        self.writer.rule(
            'bin',
            command='$objcopy -O binary $in $out',
            description='BIN $out',
        )
        self.writer.newline()
        self.writer.rule(
            'hex',
            command='$objcopy -O ihex $in $out',
            description='HEX $out',
        )
        self.writer.newline()