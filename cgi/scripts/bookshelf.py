import cgi
import os
import json

class SimpleWebApp:
    def __init__(self):
        self.books_file_path = "cgi/scripts/books.json"
        self.load_books()

    def load_books(self):
        if os.path.exists(self.books_file_path):
            with open(self.books_file_path, "r") as file:
                self.books = json.load(file)
        else:
            self.books = []

    def save_books(self):
        with open(self.books_file_path, "w") as file:
            json.dump(self.books, file)

    def render_html(self):
        html_content = """
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>Bookshelf</title>
        </head>
        <body>
            <h1>Bookshelf</h1>

            <h2>Books:</h2>
            <ul>
        """

        for book in self.books:
            html_content += f"<li>{book['title']} by {book['author']}</li>"

        html_content += """
            </ul>

            <h2>Add New Book:</h2>
            <form action="bookshelf.py" method="get">
                <label for="title">Title:</label>
                <input type="text" id="title" name="title" required>
                <br>
                <label for="author">Author:</label>
                <input type="text" id="author" name="author" required>
                <br>
                <button type="submit">Add Other</button>
            </form>
        </body>
        </html>
        """

        return html_content

    def add_book(self, new_title, new_author):
        if new_title and new_author:
            new_book = {'title': new_title, 'author': new_author}
            self.books.append(new_book)
            self.save_books()

    def run(self):
        # Simulate adding a book without user input
        if "REQUEST_METHOD" in os.environ and os.environ["REQUEST_METHOD"] == "GET":
            form = cgi.FieldStorage()
            title = form.getvalue('title', '')
            author = form.getvalue('author', '')
            self.add_book(title, author)

        html_content = self.render_html()
        print(html_content)

if __name__ == '__main__':
    web_app = SimpleWebApp()
    web_app.run()
