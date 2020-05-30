#!/usr/bin/env python3
import os

source_extensions = {'.c': 'gcc', '.s': 'as', '.cpp': 'g++'}
extensions_order = {'.s': 0, '.c': 1, '.cpp': 2}


def is_source_file(filename):
    parts = os.path.splitext(filename)
    return parts[-1] in source_extensions


def get_object_name(filename):
    parts = os.path.splitext(filename)
    return parts[0] + '.o'


def ext_pred(src):
    parts = os.path.splitext(src)
    return extensions_order.get(parts[-1])


def scan_sources():
    src = sorted([f for f in os.listdir('.') if is_source_file(f)], key=ext_pred)
    hdr = [f for f in os.listdir('.') if f.endswith('.h')]
    return src, hdr


def get_compiler(src):
    parts = os.path.splitext(src)
    return source_extensions.get(parts[-1])


def get_flags(src):
    if src.endswith('.s'):
        return ''
    return '-c -Wall -O2 -nostdlib -nostartfiles -ffreestanding'


def generate_memory_map():
    with open('memory_map', 'w') as f:
        f.write('MEMORY\n{\n\tram : ORIGIN = 0x8000, LENGTH = 0x1000000\n}\n\n')
        f.write('SECTIONS\n{\n\t.text : { *(.text*) } > ram\n')
        f.write('\t.bss : { *(.bss*) } > ram\n}\n\n')


def generate(f, project, sources, headers):
    arm = 'arm-none-eabi'
    generate_memory_map()
    objects = [os.path.join('.intr', get_object_name(src)) for src in sources]
    f.write(f'kernel.img: memory_map {" ".join(objects)}\n')
    f.write(f'\t{arm}-ld {" ".join(objects)} -T memory_map -o {project}.elf\n')
    f.write(f'\t{arm}-objcopy {project}.elf -O binary kernel.img\n\n')
    f.write(f'list:\n\t{arm}-objdump -D {project}.elf > {project}.list')
    f.write('\n')
    for src in sources:
        obj = os.path.join('.intr', get_object_name(src))
        compiler = get_compiler(src)
        flags = get_flags(src)
        f.write(f'{obj}: {src} {" ".join(headers)}\n')
        f.write(f'\t{arm}-{compiler} {flags} {src} -o {obj}\n\n')
    f.write('clean:\n\trm -f .intr/* kernel.img\n\n')


def main():
    parts = os.path.split(os.getcwd())
    project = parts[-1]
    sources, headers = scan_sources()
    with open('Makefile', 'w') as f:
        generate(f, project, sources, headers)


if __name__ == '__main__':
    try:
        os.mkdir('.intr')
    except FileExistsError:
        pass
    main()
