Name:       ast
Version:    0.0.1
Release:    1
Summary:    bla-bla
Group:      Base
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: ast.manifest

BuildRequires: gcc
BuildRequires: cmake
BuildRequires: llvm
BuildRequires: llvm-devel
BuildRequires: llvm-static-devel
BuildRequires: clang
BuildRequires: clang-devel
BuildRequires: pkgconfig(dlog)

%description
bla-bla

%define install_prefix /usr

%prep
%setup -q

%build
cp %{SOURCE1001} .

mkdir -p build
cd build

CMAKE_FLAG="$CMAKE_FLAG -DCMAKE_BUILD_TYPE=DEBUG"

cmake .. $CMAKE_FLAG

make

%install
mkdir -p %{buildroot}%{install_prefix}/bin
cp build/bin/ast_dump %{buildroot}%{install_prefix}/bin

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{install_prefix}/bin/ast_dump
