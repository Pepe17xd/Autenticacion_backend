# Astra Identity Service

Microservicio REST de identidad escrito en C++17. Proporciona registro e inicio de sesión de usuarios con PostgreSQL, contraseñas protegidas con Argon2id y tokens JWT. También incluye el inicio de la integración con Google OAuth.

## Arquitectura

```
Cliente HTTP -> Drogon (puerto 9000) -> AuthService -> PostgreSQL (puerto interno 5432)
                                      -> Argon2id / JWT / Google OAuth
```

Docker Compose crea una red privada para los servicios. PostgreSQL guarda sus datos en el volumen nombrado `identity_postgres_data`; por tanto, los datos sobreviven a `docker compose down` y al siguiente `docker compose up`.

## Tecnologías

- C++17, CMake y Ninja
- Drogon (API HTTP)
- PostgreSQL 16
- vcpkg para dependencias C++
- OpenSSL, Argon2id y JWT
- Docker y Docker Compose

## Estructura

```
src/controllers/   Rutas y respuestas HTTP
src/services/      Reglas de autenticación
src/repositories/  Acceso a PostgreSQL
src/security/      Hash de contraseñas y JWT
src/oauth/         Integración Google OAuth
migrations/        Esquema inicial de PostgreSQL
```

## Requisitos

- Docker Engine con Docker Compose v2
- Git (para clonar el proyecto)

Para compilar fuera de Docker también se necesita CMake 3.20+, un compilador C++17 y vcpkg.

## Configuración

Copie el archivo de ejemplo y complete los valores:

```powershell
Copy-Item .env.example .env
```

| Variable | Descripción |
| --- | --- |
| `POSTGRES_DB` | Nombre de la base de datos. |
| `POSTGRES_USER` | Usuario de PostgreSQL. |
| `POSTGRES_PASSWORD` | Contraseña fuerte para PostgreSQL. |
| `JWT_SECRET` | Secreto aleatorio de al menos 32 caracteres para firmar JWT. |
| `DATABASE_URL` | Cadena libpq. Debe usar el mismo usuario, contraseña y base de datos; en Compose el host es `postgres`. |
| `GOOGLE_CLIENT_ID` | Opcional; ID de cliente OAuth de Google. |
| `GOOGLE_CLIENT_SECRET` | Opcional; secreto OAuth de Google. |
| `GOOGLE_REDIRECT_URI` | Opcional; URI de retorno registrada en Google. |

`.env` está excluido de Git. No reutilice los valores de ejemplo en producción.

## Ejecutar con Docker

Desde la raíz del repositorio:

```powershell
docker compose up --build
```

La API queda disponible en `http://localhost:9000`. Para ejecutarla en segundo plano, añada `-d`. Las migraciones se aplican solo al crear el volumen por primera vez.

Ver estado y logs:

```powershell
docker compose ps
docker compose logs -f identity-service
```

Detener sin borrar datos:

```powershell
docker compose down
```

Eliminar también la base de datos persistida (acción irreversible):

```powershell
docker compose down -v
```

## Puertos

| Servicio | Puerto expuesto | Uso |
| --- | --- | --- |
| `identity-service` | 9000 | API HTTP |
| `postgres` | No expuesto al host | Disponible solo para los contenedores Compose en 5432 |

## Endpoints

| Método | Ruta | Descripción |
| --- | --- | --- |
| POST | `/api/auth/register` | Registra un usuario. |
| POST | `/api/auth/login` | Inicia sesión y devuelve un token. |
| GET | `/api/auth/google/url` | Devuelve la URL de autorización de Google si está configurado. |
| POST | `/api/auth/google/callback` | Reservado; actualmente responde `501`. |

Ejemplo de registro:

```powershell
curl.exe -X POST http://localhost:9000/api/auth/register `
  -H "Content-Type: application/json" `
  -d '{"name":"Ada Lovelace","email":"ada@example.com","password":"password-seguro"}'
```

## Compilación local opcional

```powershell
cmake -S . -B build -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release --parallel
```
