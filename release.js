const { execSync } = require('child_process');
const dotEnv = require('dotenv');
const { commandsList } = require('./commandsList');

dotEnv.config({
    path: process.env.NODE_ENV === 'test' ? '.env.test' : '.env'
});

try {
    if (!process.env.GH_TOKEN) {
        throw new Error('GH_TOKEN não encontrado no arquivo .env');
    }
    console.log('Iniciando processo de release...');
    const commands = Object.values(commandsList);
    for (const command of commands) {
        execSync(command, {
            stdio: 'inherit',
            env: {
                ...process.env,
                GH_TOKEN: process.env.GH_TOKEN
            }
        });
    }
    console.log('Release concluído com sucesso!');
} catch (error) {
    console.error('Erro durante o processo de release:', error.message);
    process.exit(1);
} 