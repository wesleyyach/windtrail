# WindTrail

**WindTrail** é um efeito nativo do KWin que desenha uma fita de vento suave e responsiva à velocidade atrás do cursor no KDE Plasma Wayland.

Diferente dos rastros clássicos que repetem imagens do cursor, o WindTrail cria uma fita curva contínua cuja largura e duração reagem à velocidade do movimento.

Recursos

- Fita curva contínua, sem repetir imagens do cursor
- Espessura e duração reagem à velocidade do mouse
- Presets Crimson Slash, Wind White, Ice Blue e cor personalizada
- Ajustes de espessura, intensidade, duração, velocidade mínima e suavidade
- Prévia e botão de configuração nativos nos Efeitos da Área de Trabalho
- Opção de desativar automaticamente em jogos e apps em tela cheia
- Suporte a múltiplos monitores
- Sem rede, telemetria ou serviço em segundo plano

## Compatibilidade

- KDE Plasma / KWin **6.7 ou superior**
- Sessão Wayland
- Composição OpenGL
- Linux

A versão 1.0.0 foi validada no **Fedora 44, Plasma 6.7.3, KWin 6.7.3, Wayland, x86_64**. Como este é um plugin nativo do KWin, pode ser necessário recompilá-lo após atualizações do KWin.

## Instalação

Baixe e extraia o arquivo da versão. Depois execute:

```bash
cd windtrail-1.0.0
./scripts/install.sh
```

No Fedora 44, instale as dependências com:

```bash
sudo dnf install \
  gcc-c++ cmake extra-cmake-modules ninja-build \
  qt6-qtbase-devel \
  kf6-kcoreaddons-devel kf6-kconfig-devel kf6-kcmutils-devel \
  kwin-devel libepoxy-devel libdrm-devel
```

## Configuração

Abra:

**Configurações do Sistema → Aplicativos e Janelas → Gerenciamento de Janelas → Efeitos da Área de Trabalho → WindTrail**

## Verificação

```bash
./scripts/verify.sh
```

## Remoção

```bash
./scripts/uninstall.sh
```

Para apagar também as configurações salvas:

```bash
./scripts/uninstall.sh --purge-settings
```

## Relatar problemas

Abra uma issue em <https://github.com/wesleyyach/windtrail/issues> e inclua a saída de:

```bash
./scripts/verify.sh
journalctl --user -b --no-pager | grep -iE 'windtrail|proxyx_windtrail|kwin'
```

## Licença

GPL-3.0-or-later. Consulte [LICENSE](LICENSE).
