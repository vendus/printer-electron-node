const { execSync } = require('child_process');
const dotEnv = require('dotenv')
dotEnv.config({
    path: process.env.NODE_ENV === 'test' ? '.env.test' : '.env'
});


try {
    if (!process.env.GH_TOKEN) {
        throw new Error('GH_TOKEN não encontrado no arquivo .env');
    }
    console.log('Iniciando processo de release...');
    execSync(`cross-env GH_TOKEN=${process.env.GH_TOKEN} semantic-release --no-ci`, {
        stdio: 'inherit'
    });
    console.log('Release concluído com sucesso!');
} catch (error) {
    console.error('Erro durante o processo de release:', error.message);
    process.exit(1);
} 