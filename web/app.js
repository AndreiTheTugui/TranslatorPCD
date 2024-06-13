function uploadFile() {
    document.getElementById('fileInput').click();
}

function handleFileUpload(event) {
    const uploadArea = document.getElementById('uploadArea');
    const optionsArea = document.getElementById('optionsArea');
    const uploadText = document.getElementById('uploadText');
    const fileIcon = document.getElementById('fileIcon');
    const fileName = document.getElementById('fileName');
    const filePreview = document.getElementById('filePreview');
    const fileInput = document.getElementById('fileInput');
    
    if (event.target.files.length > 0) {
        const file = event.target.files[0];
        const fileType = file.type;

        if (fileType === 'application/pdf' || fileType === 'application/vnd.openxmlformats-officedocument.wordprocessingml.document' || fileType === 'application/msword' || fileType === 'text/plain') {
            fileName.textContent = file.name;

            if (fileType === 'application/pdf') {
                filePreview.src = 'formats/pdf.png';
            } else if (fileType === 'application/vnd.openxmlformats-officedocument.wordprocessingml.document' || fileType === 'application/msword') {
                filePreview.src = 'formats/docx.png';
            } else if (fileType === 'text/plain') {
                filePreview.src = 'formats/txt.png';
            }

            fileIcon.style.display = 'flex';
            uploadText.style.display = 'none';
            uploadArea.style.width = '250px';
            uploadArea.style.height = '250px';
            uploadArea.style.float = 'left';
            uploadArea.style.marginRight = '20px';
            uploadArea.classList.add('disabled');
            optionsArea.style.display = 'flex';

            const reader = new FileReader();
            reader.onload = function(event) {
                const fileContent = event.target.result;
                translateFile(fileContent)
                    .then(translatedContent => {
                        console.log(translatedContent);
                    })
                    .catch(error => {
                        alert('Translation failed: ' + error);
                    });
            };
            reader.readAsText(file);

        } else {
            alert('Invalid file format. Please upload a PDF, DOCX, or TXT file.');
            fileInput.value = '';
        }
    }
}

function resetUpload(event) {
    event.stopPropagation();

    const uploadArea = document.getElementById('uploadArea');
    const uploadText = document.getElementById('uploadText');
    const fileIcon = document.getElementById('fileIcon');
    const fileInput = document.getElementById('fileInput');
    const optionsArea = document.getElementById('optionsArea');

    fileInput.value = '';
    fileIcon.style.display = 'none';
    uploadText.style.display = 'block';
    uploadArea.style.width = '500px';
    uploadArea.style.height = '400px';
    uploadArea.style.float = 'none';
    uploadArea.style.marginRight = '0';
    uploadArea.classList.remove('disabled'); 
    optionsArea.style.display = 'none';
}

function generateFile() {
    const generateButton = document.getElementById('generateButton');
    const downloadButton = document.getElementById('downloadButton');
    
    setTimeout(() => {
        generateButton.style.marginRight = '10px';
        downloadButton.style.display = 'inline-block';
    }, 1000);
}

function downloadFile() {
    alert('File ready for download!');
}

function hideDownloadButton() {
    const downloadButton = document.getElementById('downloadButton');
    downloadButton.style.display = 'none';
}

function translateFile(fileContent) {
    return new Promise((resolve, reject) => {
        const xhr = new XMLHttpRequest();
        xhr.open('POST', 'http://localhost:8888', true);
        xhr.setRequestHeader('Content-Type', 'text/plain');
        xhr.onreadystatechange = function () {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    resolve(xhr.responseText);
                } else {
                    reject(xhr.statusText);
                }
            }
        };
        xhr.send(fileContent);
    });
}
